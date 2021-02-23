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

	buf_init();

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
		buf_clear();  //reset buffer

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

		numWrites = 0; //reset number of writes

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
			digitalToggleFast(LED_BUILTIN);

			printIdle = 0; 

			dataFile.write((const uint8_t *)(cBuf + cBufWriteIdx), sizeof(struct printLine));

			#ifdef SERIAL_DEBUG 
			printCBuf(cBuf + cBufWriteIdx);
			#endif

			numWrites++;

			print_ready_flag = 0;  //reset print ready flag to allow cBufWriteIdx update
			printIdle = 1;
		}
		break;
	}
	case CLOSE:
	{
		adcTimer.end();

		Serial.println(PSTR("Halted Data Collection"));
		Serial.println(PSTR("wrapping up file..."));

		// int finalOffset = cBufWriteIdx;

		// debug(PSTR("before: "), finalOffset);

		// finalOffset -= SERIAL_BUF_DISP;

		// if (finalOffset < 0)
		// {
		// 	finalOffset = 0;
		// }
		// debug(PSTR("finalOffset: "), finalOffset);

		// Serial.println(PSTR("Flushing Write Buffer"));

		// for (int i = offset; i < PRINT_BUF_MULT; i++)
		// {
		// 	rBuf->time[i] = 0; //zero fill rest of data
		// }

		// debug("offset: ", offset);

		//TODO: need to fix this so that you write out rest of buffered data
		//		the issue now is that you could be forgetting to write data in overflow buffer

		// dataFile.write((const uint8_t *)rBuf, sizeof(printBuf)); //write out rest of read buffer

		numWrites++;

		dataFile.close();

		debug("number of errors: ", numErrors);

		logger_state = IDLE;

		break;
	}
	}
}
