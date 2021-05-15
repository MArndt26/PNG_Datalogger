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
#include "commands/commands.h"

volatile int print_ready_flag = 0;
volatile int sd_print_comp_flag = 1;
volatile int print_overflow_flag = 0;

int numWrites = 0;
int numErrors = 0;

elapsedMicros time;

unsigned int startTime;

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

	command_shell();
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
		buf_clear(); //reset buffer

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
		startTime = time;
		logger_state = WRITE;
		break;
	}
	case WRITE:
	{
		if (writeReady())
		{
			digitalToggleFast(LED_BUILTIN);

			dataFile.write(write(), sizeof(printBuf));

#ifdef SERIAL_DEBUG
			printCBuf(cBuf + cBufWriteIdx);
#endif

			numWrites++;

			nextwrite();
		}
		break;
	}
	case CLOSE:
	{
		adcTimer.end();

		sd_wrap_up();

		debug("number of errors: ", numErrors);

		logger_state = IDLE;

		break;
	}
	}
}
