#include <Arduino.h>

#include "serial.h"
#include "state.h"

extern STATE curState;

void serialEvent()
{
    char rx = 0;
    while (Serial.available()) //get most recent byte sent
    {
        rx = (char)Serial.read();
    }

    switch (rx)
    {
    case START:
        curState = RUNNING;
        break;
    case HALT:
        curState = IDLE;
        break;
    case CREATE_FILE:
        break;
    }
}