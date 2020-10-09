/*
 * Blink
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

#include <Arduino.h>

void blink(int num, int dur);

void setup()
{
	// initialize LED digital pin as an output.
	pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
	blink(3, 150);

	blink(1, 1000);
}

void blink(int num, int dur)
{
	for (int i = 0; i < num; i++)
	{
		digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
		delay(dur);						 // wait for a second
		digitalWrite(LED_BUILTIN, LOW);	 // turn the LED off by making the voltage LOW
		delay(dur);						 // wait for a second
	}
}
