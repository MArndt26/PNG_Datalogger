#include <SD.h>
#include <SPI.h>

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
	OPEN_FILE,
	AWAIT,
	WRITE,
	CLOSE_FILE
};

//private variables
//----------------STATE VARS-----------------//
STATE curState = IDLE;

//----------------TIMER VARS-----------------//
volatile int isrCount = 0;

IntervalTimer timer; // Create an IntervalTimer object

volatile int writeReady = 0;

const int period = 500; //period in microseconds

//---------------BUFFER VARS----------------//
String dataString = "";

//---------------TIME VARS------------------//

volatile int prevIsrCount = -1;

int time = 0;

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
	if (!SD.begin(BUILTIN_SDCARD))
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
	switch (curState)
	{
	case IDLE:
		break;
	case CREATE_FILE:
		break;
	case FILE_LOADED:
		break;
	case OPEN_FILE:
		break;
	case AWAIT:
		break;
	case WRITE:
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
	case CLOSE_FILE:
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

void serialEvent()
{
	char rx = 0;
	while (Serial.available()) //get most recent byte sent
	{
		rx = (char)Serial.read();
	}

	switch (rx)
	{
	case START_CMD:
		curState = AWAIT;
		break;
	case HALT_CMD:
		curState = IDLE;
		break;
	case CREATE_FILE_CMD:
		break;
	}
}