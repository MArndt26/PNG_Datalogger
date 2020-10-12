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

void blink(int times, int d);

ADC *adc = new ADC();

const int ADC_CHAN = 10;

const int MUXED_CHAN = 6;

uint8_t adc_pins[] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9};

const int chipSelect = BUILTIN_SDCARD;

const int numChannels = 10;

File dataFile;
char fName[10];

int numWrites = 0;

elapsedMicros time;

uint16_t datastore[ADC_CHAN * MUXED_CHAN];

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

		numWrites = 0;

		break;
	}
	case FILE_LOADED:
	{
		blink(1, 500);
		time = 0; //clear timestamp
		break;
	}
	case WRITE:
	{
		digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN));

		unsigned int tempTime = time;

		dataFile.write((const uint8_t *)&tempTime, sizeof(tempTime));

		for (int j = 0; j < MUXED_CHAN; j++)
		{
			for (int i = 0; i < ADC_CHAN; i++)
			{
				datastore[ADC_CHAN * j + i] = adc->analogRead(adc_pins[i]);
			}
		}

		dataFile.write((const uint8_t *)&datastore, sizeof(datastore));
		numWrites++;
		break;
	}
	case CLOSE:
	{
		int tmpTime = time;
		Serial.println("Halted Data Collection");
		dataFile.close();
		Serial.print("Number of writes: ");
		Serial.println(numWrites);
		Serial.print("time: ");
		Serial.println(tmpTime);
		Serial.print("Write Freq: ");
		Serial.println(numWrites / (time / 1000000.0));

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