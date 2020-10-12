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

ADC *adc = new ADC();

#define PINS 10

uint8_t adc_pins[] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9};

const int chipSelect = BUILTIN_SDCARD;

const int numChannels = 10;

File dataFile;

int numWrites = 0;

unsigned int time = 0;

uint16_t datastore[10];

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);

	for (int i = 0; i < PINS; i++)
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

	SD.remove("datalog.txt"); //remove previous versions of file

	dataFile = SD.open("datalog.txt", FILE_WRITE);

	time = millis();
}

int value = 0;
int pin = 0;
void loop()
{
	for (int i = 0; i < PINS; i++)
	{
		datastore[i] = adc->analogRead(adc_pins[i]);
	}
	dataFile.write((const uint8_t *)&datastore, sizeof(datastore));
	numWrites++;
}

void serialEvent()
{
	time = millis() - time;
	Serial.println("Halted Data Collection");
	dataFile.close();
	Serial.print("Number of writes: ");
	Serial.println(numWrites);
	Serial.print("time: ");
	Serial.println(time);
	Serial.print("Write Freq: ");
	Serial.println(numWrites * 1000 / time);
	Serial.println("Endless Loop");
	while (true)
		;
}