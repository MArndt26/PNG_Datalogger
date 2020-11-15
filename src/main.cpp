/*
  SD card datalogger

 This example shows how to log data from three analog sensors
 to an SD card using the SD library.

 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11, pin 7 on Teensy with audio board
 ** MISO - pin 12
 ** CLK - pin 13, pin 14 on Teensy with audio board
 ** CS - pin 4,  pin 10 on Teensy with audio board

 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */
#include "main.h"
#include "png_adc.h"
#include "png_buf.h"
#include "png_mux.h"
#include "png_sd.h"
#include "png_serial.h"
#include "png_states.h"
#include "png_sync.h"

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);

	pinMode(SYNC_PIN, OUTPUT);

	for (int i = 0; i < ADC_CHAN; i++)
	{
		pinMode(adc_pins[i], INPUT);
	}

	for (unsigned int i = 0; i < sizeof(mux_pins) / sizeof(mux_pins[0]); i++)
	{
		pinMode(mux_pins[i], OUTPUT);
		digitalWriteFast(mux_pins[i], LOW); //initialize to low
	}

	///// ADC0 ////
	adc->adc0->setAveraging(1);											  // set number of averages
	adc->adc0->setResolution(12);										  // set bits of resolution
	adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED); // change the conversion speed
	adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);	  // change the sampling speed

////// ADC1 /////
#ifdef ADC_DUAL_ADCS
	adc->adc1->setAveraging(1);											  // set number of averages
	adc->adc1->setResolution(12);										  // set bits of resolution
	adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED); // change the conversion speed
	adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);	  // change the sampling speed
#endif

	// Open serial communications and wait for port to open:
	Serial.begin(9600);
	while (!Serial)
		; // wait for serial port to connect.

	Serial.print(PSTR("Initializing SD card..."));

	// see if the card is present and can be initialized:
	if (!SD.begin(chipSelect))
	{
		Serial.println(PSTR("Card failed, or not present"));
		// don't do anything more:
		return;
	}

	Serial.println(PSTR("card initialized."));
}

int value = 0;
int pin = 0;
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
