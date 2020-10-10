#include <SD.h>
#include <SPI.h>
#include <Snooze.h>

//private function declarations
void timerISR();

void serialEvent();

//private enumerations
enum SERIAL_CMD
{
	CREATE_FILE_CMD = 'c',
	START_CMD = 's',
	HALT_CMD = 'h',
};

enum STATE
{
	IDLE,
	CREATE_FILE,
	FILE_LOADED,
	PREPARE,
	AWAIT,
	WRITE,
	CLOSE
};

//private variables
//----------------STATE VARS-----------------//
STATE curState = IDLE;

//----------------SNOOZE VARS----------------//
SnoozeTimer sleepTimer;
SnoozeUSBSerial sleepUSB;

SnoozeBlock config(sleepTimer, sleepUSB);

//----------------TIMER VARS-----------------//
volatile int isrCount = 0;

IntervalTimer timer; // Create an IntervalTimer object

const int period = 250; //period in microseconds

//---------------BUFFER VARS----------------//
String dataString = "";
const int writeBufSize = 10;
int writeBuf[writeBufSize];

//---------------TIME VARS------------------//

volatile int prevIsrCount = -1;

unsigned long time = 0;

//---------------FILE VARS------------------//
File dataFile;
char fName[10];

//---------------COUNT VARS-----------------//
volatile int numWrite = 0;
volatile int numConversions = 0;

//---------------LED VARS--------------------//
int ledstate = 0;

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);
	// Open serial communications and wait for port to open:
	Serial.begin(9600);
	while (!Serial)
		; // wait for serial port to connect.

	Serial.print("Initializing SD card...");

	// see if the card is present and can be initialized:
	if (!SD.begin(BUILTIN_SDCARD))
	{
		Serial.println("Card failed, or not present");
		// don't do anything more:
		return;
	}
	Serial.println("card initialized.");

	// sleepTimer.setTimer(period / 1000); //milliseconds
}

void loop()
{
	switch (curState)
	{
	case IDLE:
	{
		// Serial.println("Entering IDLE...");
		// Snooze.deepSleep(config);
		digitalWrite(LED_BUILTIN, ledstate);
		ledstate = !ledstate;
		delay(500);
		asm volatile("wfi");
		break;
	}
	case CREATE_FILE:
	{
		Serial.println("Creating File...");
		int fNum = -1;
		do
		{
			fNum++;
			sprintf(fName, "F%d.txt", fNum);
		} while (SD.exists(fName));

		Serial.print("Filename Created: ");
		Serial.println(fName);

		curState = FILE_LOADED;
		break;
	}
	case FILE_LOADED:
	{
		asm volatile("wfi");
		break;
	}
	case PREPARE:
	{
		Serial.println("Preparing File for Run...");
		dataFile = SD.open(fName, FILE_WRITE); //create file

		if (!dataFile)
		{
			Serial.println("error opening file");
			curState = IDLE;
			break;
		}

		// timer.begin(timerISR, period); //start data collection timer

		Serial.println("Timer Started...");
		time = millis();
		curState = AWAIT;
		break;
	}
	case AWAIT:
	{
		curState = WRITE;
		break;
	}
	case WRITE:
	{
		// dataString = "";
		// read three sensors and append to the string:
		for (int analogPin = 0; analogPin < 10; analogPin++)
		{
			int sensor = analogRead(analogPin);
			dataFile.print(sensor);
			writeBuf[analogPin] = sensor;
			// dataString += String(sensor);

			// dataString += ",";
		}
		// curState = WRITE;
		// numConversions++;

		// dataFile.println(dataString);
		// dataFile.print(writeBuf, 10);
		numWrite++;

		break;
	}
	case CLOSE:
	{
		Serial.println("Closing File...");
		// timer.end(); //stop data collection timer
		dataFile.close();

		time = millis() - time;
		Serial.print("Write Time: ");
		Serial.println(time);

		Serial.println("File Closed:");
		Serial.print("numWrite: ");
		Serial.println(numWrite);
		Serial.print("numConversions: ");
		Serial.println(numConversions);
		Serial.print("Write Freq: ");
		Serial.println(numWrite * 1000 / time);
		Serial.print("Conversion Freq: ");
		Serial.println(numConversions * 10 * 1000 / time);
		curState = IDLE;
		break;
	}
	}
}

// void timerISR()
// {

// }

void serialEvent()
{
	char rx = 0;
	while (Serial.available()) //get most recent byte sent
	{
		rx = (char)Serial.read();
	}

	Serial.println(rx);

	switch (rx)
	{
	case CREATE_FILE_CMD:
		if (curState == IDLE)
		{
			curState = CREATE_FILE;
		}
		break;
	case START_CMD:
		if (curState == FILE_LOADED)
		{
			curState = PREPARE;
		}
		break;
	case HALT_CMD:
		if (curState == AWAIT || curState == WRITE)
		{
			curState = CLOSE;
		}
		break;
	}
}