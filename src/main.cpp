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

#include <SD.h>
#include <SPI.h>
#include <ADC.h>
#include <ADC_util.h>

#include "helpers.h"

#define SERIAL_DEBUG
#define SAMPLING_PERIOD 1000

const int SERIAL_BUF_DISP = 5;

void printPBuf(int offset);

void adc_isr();

void error(const String msg);

IntervalTimer adcTimer;

ADC *adc = new ADC();

volatile int adc_ready_flag = 0;
volatile int sd_print_comp_flag = 1;

const int ADC_CHAN = 10;

const int MUXED_CHAN = 6;

const uint8_t adc_pins[] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10};

const uint8_t mux_pins[] = {30, 31}; //A B

const uint8_t SEL_A = 0;
const uint8_t SEL_B = 1;

int mux_state = 0;

const int chipSelect = BUILTIN_SDCARD;

File dataFile;
char fName[10];

int numWrites = 0;
int numErrors = 0;

elapsedMicros time;

unsigned int adcTime;

const int PRINT_BUF_MULT = 1000;

struct printBuf
{
	unsigned int time[PRINT_BUF_MULT];
	uint16_t data[PRINT_BUF_MULT][ADC_CHAN * MUXED_CHAN];
} printBuf;

volatile int offset;

struct printBuf pB1;
struct printBuf pB2;
struct printBuf pBOver;

volatile int preOverflowBuffer = 0;
volatile int buf_overflow_offset = 0;

struct printBuf *wBuf = nullptr;
struct printBuf *rBuf = &pB1;

enum LOGGER_STATE
{
	IDLE,
	CREATE_FILE,
	FILE_LOADED,
	START_COLLECTION,
	WRITE,
	CLOSE
};

LOGGER_STATE logger_state = IDLE;

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);

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
#endif
		logger_state = WRITE;
		break;
	}
	case WRITE:
	{
#ifdef SERIAL_DEBUG
		adc_isr();
		debug("offset: ", offset);
		debug("overflow offset: ", buf_overflow_offset);
		delay(1000);
#endif
		if (adc_ready_flag)
		{
			adc_ready_flag = 0;

			digitalToggleFast(LED_BUILTIN);

			if (wBuf == nullptr)
			{
				error("wBuf is nullptr");
			}

			dataFile.write((const uint8_t *)wBuf, sizeof(printBuf));

			wBuf = nullptr; //clear write buffer

			if (buf_overflow_offset > 0)
			{
				for (int i = buf_overflow_offset; i < PRINT_BUF_MULT; i++)
				{
					pBOver.time[i] = 0; //signify junk data with zeros for time
										// for (int j = 0; j < MUXED_CHAN * ADC_CHAN; j++)
										// {
										// 	pBOver.data[i][j] = 0;
										// }
				}
				dataFile.write((const uint8_t *)&pBOver, sizeof(printBuf));
				buf_overflow_offset = 0;
			}

			numWrites++;

#ifdef SERIAL_DEBUG
			debug(PSTR("offset: "), offset);
			int tempOffset = offset;
			tempOffset -= SERIAL_BUF_DISP;
			if (tempOffset < 0)
			{
				tempOffset = 0;
			}
			printPBuf(tempOffset);

			delay(500);
#endif
		}
		break;
	}
	case CLOSE:
	{
		int tmpTime = time;

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

		// time = time - tmpTime;
		while (1)
		{
			for (int j = 0; j < MUXED_CHAN; j++)
			{
				for (int i = 0; i < ADC_CHAN; i++)
				{
					rBuf->data[offset][ADC_CHAN * j + i] = 0;
				}
				rBuf->time[offset] = 0;
				offset++;
				if (offset >= PRINT_BUF_MULT)
				{
					goto END_WHILE;
				}
			}
		}

	END_WHILE:

		dataFile.write((const uint8_t *)rBuf, sizeof(printBuf)); //write out rest of read buffer

		numWrites++;

		dataFile.close();
		debug(PSTR("Number of errors: "), numErrors);
		debug(PSTR("Number of writes: "), numWrites);
		debug(PSTR("time: "), tmpTime);
		debug(PSTR("Avg Write Freq: "), (numWrites * PRINT_BUF_MULT) / (tmpTime / 1000000.0));
		Serial.println(PSTR("Time Deltas"));
		for (int i = finalOffset + 1; i < finalOffset + SERIAL_BUF_DISP; i++)
		{
			int delta = wBuf->time[i] - wBuf->time[i - 1];
			if (delta < 0 || delta == 0)
			{
				Serial.print(PSTR("NA,"));
			}
			else
			{
				Serial.print(delta);
				Serial.print(PSTR(","));
			}
		}
		Serial.println();

		Serial.println("last pBuf: ");
		printPBuf(finalOffset);
		logger_state = IDLE;

		break;
	}
	}
}

void serialEvent()
{
	char c = Serial.read();

	Serial.println(c);

	switch (c)
	{
	case 'c':
		if (logger_state == IDLE)
		{
			logger_state = CREATE_FILE;
		}
		break;
	case 's':
		if (logger_state == FILE_LOADED)
		{
			logger_state = START_COLLECTION;
		}
		break;
	case 'h':
		if (logger_state == WRITE)
		{
			logger_state = CLOSE;
		}
		break;
	}
}

void adc_isr()
{
	digitalToggle(LED_BUILTIN);

	if (rBuf == wBuf)
	{
		debug("Num Writes: ", numWrites);
		error(PSTR("Buffer Overrun Error!"));
	}

	volatile int *off = &offset;
	if (rBuf == &pBOver)
	{
		off = &buf_overflow_offset; //change offset for overflow case
	}

	for (int j = 0; j < MUXED_CHAN; j++)
	{
		/**
			 * MUX LOGIC (CD4052)
			 * 	31	30     <----MCU output pins
			 * 	B	A	OUT	
			 * 	0	0	0
			 * 	0	1	1
			 * 	1	0	2
			 * 	1	1	3
			 */
		switch (mux_state)
		{
		case 0:
		{
			digitalWriteFast(mux_pins[SEL_A], LOW);
			digitalWriteFast(mux_pins[SEL_B], LOW);
			break;
		}
		case 1:
		{
			digitalWriteFast(mux_pins[SEL_A], HIGH);
			digitalWriteFast(mux_pins[SEL_B], LOW);
			break;
		}
		case 2:
		{
			digitalWriteFast(mux_pins[SEL_A], LOW);
			digitalWriteFast(mux_pins[SEL_B], HIGH);
			break;
		}
			// case 3:
			// {
			// 	digitalWriteFast(mux_pins[SEL_A], HIGH);
			// 	digitalWriteFast(mux_pins[SEL_B], HIGH);
			// 	mux_state = 0; //wrap around
			// 	break;
			// }
		}
		mux_state++;
		if (mux_state > 2)
		{
			mux_state = 0; //wrap around
		}

		//read in adc channels
		for (int i = 0; i < ADC_CHAN; i++)
		{
			rBuf->data[*off][ADC_CHAN * j + i] = adc->analogRead(adc_pins[i]);
		}
	}

	rBuf->time[*off] = time;

	(*off)++;
	if (*off >= PRINT_BUF_MULT)
	{
		if (rBuf == &pBOver)
		{
			error(PSTR("Overflowed the overflow buffer"));
		}

		if (wBuf == nullptr)
		{
			wBuf = rBuf; //set write buffer
		}
		else
		{
			numErrors++;
			// offset--;
			if (rBuf == &pB1)
			{
				preOverflowBuffer = 1;
			}
			else if (rBuf == &pB2)
			{
				preOverflowBuffer = 2;
			}
			else
			{
				error(PSTR("Invalid rBuf before overflow"));
			}

			rBuf = &pBOver; //set overflow buffer;
			return;
			// error(PSTR("Tried to overwrite non null wBuf"));
		}

		offset = 0; //wrap around buffer;

		if (rBuf == &pB1)
		{
			rBuf = &pB2;
		}
		else if (rBuf == &pB2)
		{
			rBuf = &pB1; //wrap around case
		}
		else
		{
			error(PSTR("Invalid Read Buffer"));
		}
		adc_ready_flag = 1;
	}
	else if (rBuf == &pBOver && wBuf == nullptr)
	{
		//write complete in offset case

		if (preOverflowBuffer == 1)
		{
			wBuf = &pB1;
			rBuf = &pB2;
		}
		else if (preOverflowBuffer == 2)
		{
			wBuf = &pB2;
			rBuf = &pB1;
		}
		else
		{
			error(PSTR("Invalid before set after overflow completion"));
		}

		adc_ready_flag = 1; //set adc_ready_flag
	}
}

void printPBuf(int offset)
{
	debug(PSTR("offset: "), offset);
	for (int j = offset; j < offset + SERIAL_BUF_DISP; j++)
	{
		Serial.print(rBuf->time[j]);

		int d = countDigits(rBuf->time[j]);

		while (d < 15)
		{
			Serial.print(PSTR(" "));
			d++;
		}

		Serial.print(',');
		for (int i = 0; i < ADC_CHAN * MUXED_CHAN; i++)
		{
			Serial.print(rBuf->data[j][i]);

			Serial.print(',');
		}
		Serial.println();
	}
}

void error(String msg)
{
	adcTimer.end();

	debug("numWrites: ", numWrites);

	Serial.println(msg);

	while (1)
	{
		blink(1, 200);
	}
}