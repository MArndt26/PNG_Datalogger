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

// #define SERIAL_DEBUG

void blink(int times, int d);

void debug(String msg, int val);

void printPBuf(int offset);

int countDigits(int n);

ADC *adc = new ADC();

const int ADC_CHAN = 10;

const int MUXED_CHAN = 6;

const uint8_t adc_pins[] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9};

const uint8_t mux_pins[] = {30, 31}; //A B

const uint8_t SEL_A = 0;
const uint8_t SEL_B = 1;

int mux_state = 0;

const int chipSelect = BUILTIN_SDCARD;

const int numChannels = 10;

File dataFile;
char fName[10];

int numWrites = 0;

elapsedMicros time;

unsigned int adcTime;

const int PRINT_BUF_MULT = 3000;

const int SERIAL_BUF_DISP = 20;

// uint16_t datastore[ADC_CHAN * MUXED_CHAN * PRINT_BUF_MULT + PRINT_BUF_MULT];

struct printBuf
{
	unsigned int time[PRINT_BUF_MULT];
	uint16_t data[PRINT_BUF_MULT][ADC_CHAN * MUXED_CHAN];
} printBuf;

int offset;

struct printBuf pBuf;

enum LOGGER_STATE
{
	IDLE,
	CREATE_FILE,
	FILE_LOADED,
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
	{
		; // wait for serial port to connect.
	}

	Serial.print("Initializing SD card...");

	// see if the card is present and can be initialized:
	if (!SD.begin(chipSelect))
	{
		Serial.println("Card failed, or not present");
		// don't do anything more:
		return;
	}

	Serial.println("card initialized.");
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
		Serial.println("Initializing Buffer...");
		for (unsigned int i = 0; i < sizeof(pBuf.time) / sizeof(pBuf.time[0]); i++)
		{
			pBuf.time[i] = 0;
		}

		for (int j = 0; j < PRINT_BUF_MULT; j++)
		{
			for (int i = 0; i < ADC_CHAN * MUXED_CHAN; i++)
			{
				pBuf.data[j][i] = 0;
			}
		}

		offset = 0;
		Serial.println("Creating File...");
		int fNum = -1;
		do
		{
			fNum++;
			sprintf(fName, "F%d.bin", fNum);
		} while (SD.exists(fName));

		Serial.print("Filename Created: ");
		Serial.println(fName);

		dataFile = SD.open(fName, FILE_WRITE);

		if (dataFile)
		{
			Serial.print("File Loaded: ");
			Serial.println(fName);
			logger_state = FILE_LOADED;
		}
		else
		{
			Serial.println("Error with Datafile: Endless loop");
			while (true)
			{
				blink(1, 1000);
			}
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
		time = 0; //clear timestamp
		adcTime = 1000;
		break;
	}
	case WRITE:
	{
		if (time - adcTime >= 900)
		{
			digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN));

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
					pBuf.data[offset][ADC_CHAN * j + i] = adc->analogRead(adc_pins[i]);
				}
			}

			pBuf.time[offset] = time;

			offset++;
			if (offset >= PRINT_BUF_MULT)
			{
				offset = 0; //wrap around buffer;
				dataFile.write((const uint8_t *)&pBuf, sizeof(pBuf));
				numWrites++;
			}

#ifdef SERIAL_DEBUG
			debug("offset: ", offset);
			int tempOffset = offset;
			tempOffset -= SERIAL_BUF_DISP;
			if (tempOffset < 0)
			{
				tempOffset = 0;
			}
			printPBuf(tempOffset);

			delay(500);
#endif
			adcTime = time;
		}
		break;
	}
	case CLOSE:
	{
		int tmpTime = time;

		Serial.println("Halted Data Collection");
		Serial.println("wrapping up file...");

		int finalOffset = offset;

		debug("before: ", finalOffset);

		finalOffset -= SERIAL_BUF_DISP;

		if (finalOffset < 0)
		{
			finalOffset = 0;
		}
		debug("finalOffset: ", finalOffset);

		// time = time - tmpTime;
		while (1)
		{
			for (int j = 0; j < MUXED_CHAN; j++)
			{
				for (int i = 0; i < ADC_CHAN; i++)
				{
					pBuf.data[offset][ADC_CHAN * j + i] = 0;
				}
				pBuf.time[offset] = time;
				offset++;
				if (offset >= PRINT_BUF_MULT)
				{
					goto END_WHILE;
				}
			}
		}

	END_WHILE:

		dataFile.write((const uint8_t *)&pBuf, sizeof(pBuf));
		numWrites++;

		dataFile.close();
		Serial.print("Number of writes: ");
		Serial.println(numWrites);
		Serial.print("time: ");
		Serial.println(tmpTime);
		Serial.print("Avg Write Freq: ");
		Serial.println(numWrites * PRINT_BUF_MULT / (time / 1000000.0));
		Serial.println("Time Deltas");
		for (int i = finalOffset + 1; i < finalOffset + SERIAL_BUF_DISP; i++)
		{
			int delta = pBuf.time[i] - pBuf.time[i - 1];
			if (delta < 0 || delta == 0)
			{
				Serial.print("NA,");
			}
			else
			{
				Serial.print(delta);
				Serial.print(",");
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
			logger_state = WRITE;
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

void blink(int times, int d)
{
	for (int i = 0; i < times; i++)
	{
		digitalWriteFast(LED_BUILTIN, HIGH);
		delay(d);
		digitalWriteFast(LED_BUILTIN, LOW);
		delay(d);
	}
}

void debug(String msg, int val)
{
	Serial.print(msg);
	Serial.println(val);
}

void printPBuf(int offset)
{
	debug("offset: ", offset);
	for (int j = offset; j < offset + SERIAL_BUF_DISP; j++)
	{
		Serial.print(pBuf.time[j]);

		int d = countDigits(pBuf.time[j]);

		while (d < 15)
		{
			Serial.print(" ");
			d++;
		}

		Serial.print(',');
		for (int i = 0; i < ADC_CHAN * MUXED_CHAN; i++)
		{
			Serial.print(pBuf.data[j][i]);

			Serial.print(',');
		}
		Serial.println();
	}
}

int countDigits(int n)
{
	if (n == 0)
	{
		return 1;
	}
	int count = 0;
	while (n != 0)
	{
		n /= 10;
		count++;
	}
	return count;
}