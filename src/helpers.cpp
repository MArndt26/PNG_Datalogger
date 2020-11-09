#include <Arduino.h>
#include "helpers.h"

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

void debug(String msg, int val)
{
    Serial.print(msg);
    Serial.println(val);
}