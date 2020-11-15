/**
 * PNG Datalogger
 * https://github.com/MArndt26/PNG_Datalogger
 * 
 * Author: Mitchell Arndt
 * created: 11/15/2020
 */
#include "main.h"
#include "png_adc.h"
#include "png_buf.h"
#include "png_mux.h"
#include "png_sd.h"
#include "png_serial.h"
#include "png_states.h"
#include "png_sync.h"

volatile int print_ready_flag = 0;
volatile int sd_print_comp_flag = 1;
volatile int print_overflow_flag = 0;

int numWrites = 0;
int numErrors = 0;

elapsedMicros time;

unsigned int adcTime;

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);

	serial_init();

	sync_init();

	mux_init();

	adc_init();

	sd_init();
}

void loop()
{
	switch (logger_state)
	{
	case IDLE:
	{
		blink(1, 100);
		break;
	}

	case CREATE_FILE:
	{
		Serial.println(PSTR("Initializing Buffer..."));
		for (unsigned int i = 0; i < sizeof(rBuf->time) / sizeof(rBuf->time[0]); i++)
		{
			rBuf->time[i] = 0;
		}

		for (int j = 0; j < PRINT_BUF_MULT; j++)
		{
			for (int i = 0; i < ADC_CHAN; i++)
			{
				rBuf->data[j][i] = 0;
			}
		}

		offset = 0;
		Serial.println(PSTR("Creating File..."));
		int fNum = -1;
		do
		{
			fNum++;
			sprintf(fName, PSTR("F%d.bin"), fNum);
		} while (SD.exists(fName));

		Serial.print(PSTR("Filename Created: "));
		Serial.println(fName);

		dataFile = SD.open(fName, FILE_WRITE);

		if (dataFile)
		{
			Serial.print(PSTR("File Loaded: "));
			Serial.println(fName);
			logger_state = FILE_LOADED;
		}
		else
		{
			error(PSTR("Error with Datafile: Endless loop"));
		}
#ifdef SERIAL_DEBUG
		for (int i = 0; i < 9; i++)
		{
			Serial.print(' ');
		}
		for (int i = 0; i < 60; i++)
		{
			Serial.print(i);
			Serial.print(',');
		}
		Serial.println();
#endif

		numWrites = 0;
		rBuf = &pB1;
		wBuf = nullptr;

		break;
	}
	case FILE_LOADED:
	{
		blink(1, 500);
		break;
	}
	case START_COLLECTION:
	{
#ifndef SERIAL_DEBUG
		adcTimer.begin(adc_isr, SAMPLING_PERIOD);
#else
		adcTimer.begin(adc_isr, SERIAL_DELAY);
#endif
		logger_state = WRITE;
		break;
	}
	case WRITE:
	{
		if (print_ready_flag)
		{
#ifdef SERIAL_DEBUG
			debugAll("inside print_ready_flag");
#endif

			print_ready_flag = 0;

			digitalToggleFast(LED_BUILTIN);

			if (wBuf == nullptr)
			{
				error("wBuf is nullptr");
			}

#ifndef SERIAL_DEBUG
			dataFile.write((const uint8_t *)wBuf, sizeof(printBuf));
#else
			printBuffer("wBuf: ", wBuf);
			printPBuf(PRINT_BUF_MULT - SERIAL_BUF_DISP, wBuf);

			do
			{
				if (Serial.available())
				{
					char c = Serial.read();

					Serial.println(c);

					if (c == 'd')
					{
						stall_print = 0;
					}
				}
			} while (stall_print); //wait for print to finish
#endif
			wBuf = nullptr; //clear write buffer

			if (print_overflow_flag)
			{
				print_overflow_flag = 0;
#ifdef SERIAL_DEBUG
				debug("buf_overflow_offset: ", buf_overflow_offset);
#endif
				for (int i = buf_overflow_offset; i < PRINT_BUF_MULT; i++)
				{
					pBOver.time[i] = 0; //signify junk data with zeros for time
				}

#ifndef SERIAL_DEBUG
				dataFile.write((const uint8_t *)&pBOver, sizeof(printBuf));
#else
				printBuffer("pBover: ", &pBOver);
				printPBuf(PRINT_BUF_MULT - SERIAL_BUF_DISP, &pBOver);

#endif
				buf_overflow_offset = 0;
			}

			numWrites++;
		}
		break;
	}
	case CLOSE:
	{
		adcTimer.end();

		Serial.println(PSTR("Halted Data Collection"));
		Serial.println(PSTR("wrapping up file..."));

		int finalOffset = offset;

		debug(PSTR("before: "), finalOffset);

		finalOffset -= SERIAL_BUF_DISP;

		if (finalOffset < 0)
		{
			finalOffset = 0;
		}
		debug(PSTR("finalOffset: "), finalOffset);

		Serial.println(PSTR("Flushing Write Buffer"));

		if (rBuf == &pBOver)
		{
			Serial.println(PSTR("rBuf is pBOver"));
		}
		else if (rBuf == &pB1)
		{
			Serial.println(PSTR("rBuf is pB1"));
		}
		else if (rBuf == &pB2)
		{
			Serial.println(PSTR("rBuf is pB2"));
		}
		else
		{
			error(PSTR("invalid rBuf in close"));
		}

		for (int i = offset; i < PRINT_BUF_MULT; i++)
		{
			rBuf->time[i] = 0; //zero fill rest of data
		}

		debug("offset: ", offset);

		dataFile.write((const uint8_t *)rBuf, sizeof(printBuf)); //write out rest of read buffer

		numWrites++;

		dataFile.close();

		debug("number of errors: ", numErrors);

		logger_state = IDLE;

		break;
	}
	}
}
