#include <SD.h>
#include <SPI.h>

#include "state.h"
#include "serial.h"

//private function declartations
void timerISR();

//private variable declarations
const int chipSelect = BUILTIN_SDCARD;

const int period = 500; //period in microseconds

IntervalTimer timer; // Create an IntervalTimer object

volatile int prevIsrCount = -1;

volatile int isrCount = 0;

volatile int writeReady = 0;

// make a string for assembling the data to log:
String dataString = "";

int time = 0;

extern STATE curState;

void setup()
{
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

	timer.begin(timerISR, period); //start data collection timer
}

void loop()
{
	if (isrCount / 1000 != prevIsrCount / 1000)
	{
		time = millis();

		Serial.print("time: ");
		Serial.println(time);

		Serial.print("isrCount: ");
		Serial.println(isrCount);
		prevIsrCount = isrCount;
	}

	switch (curState)
	{
	case IDLE:
		break;
	case RUNNING:
		if (writeReady)
		{
			writeReady = 0;
			// open the file.
			File dataFile = SD.open("datalog.txt", FILE_WRITE);

			// if the file is available, write to it:
			if (dataFile)
			{
				dataFile.println(dataString);
				dataFile.close();
				// print to the serial port too:
				Serial.println(dataString);
			}
			// if the file isn't open, pop up an error:
			else
			{
				Serial.println("error opening datalog.txt");
			}
		}
		break;
	}
}

void timerISR()
{
	isrCount++;

	dataString = "";
	// read three sensors and append to the string:
	for (int analogPin = 0; analogPin < 10; analogPin++)
	{
		int sensor = analogRead(analogPin);
		dataString += String(sensor);

		dataString += ",";
	}
	writeReady = 1;
}