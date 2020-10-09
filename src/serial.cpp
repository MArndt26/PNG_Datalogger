#include <Arduino.h>

void serialEvent()
{
    while (Serial.available())
    {
        char rx = (char)Serial.read();
        Serial.print(rx);
    }
}