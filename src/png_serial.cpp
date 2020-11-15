#include "png_serial.h"
#include "Arduino.h"
#include "png_buf.h"
#include "png_adc.h"
#include "png_mux.h"
#include "png_states.h"
#include "main.h"

int stall_print = 0;

void printPBuf(int offset, struct printBuf *buf)
{
    debug(PSTR("print offset: "), offset);

    for (int j = offset; j < offset + SERIAL_BUF_DISP; j++)
    {
        Serial.print(buf->time[j]);

        int d = countDigits(buf->time[j]);

        while (d < 15)
        {
            Serial.print(PSTR(" "));
            d++;
        }

        Serial.print(',');
        for (int i = 0; i < ADC_CHAN * MUXED_CHAN; i++)
        {
            Serial.print(buf->data[j][i]);

            Serial.print(',');
        }
        Serial.println();
    }
}

void error(String msg)
{
    adcTimer.end();

    Serial.println(msg);

    debug("numWrites: ", numWrites);

    debug("offset: ", offset);

    debug("overflow offset: ", buf_overflow_offset);

    while (1)
    {
        blink(1, 200);
    }
}

void printBufInfo()
{
    printBuffer("rBuf: ", rBuf);
    printBuffer("wBuf: ", wBuf);
}

void printBuffer(String name, struct printBuf *buf)
{
    Serial.print(name);

    int n = getBufNum(buf);
    switch (n)
    {
    case 1:
    {
        Serial.println("pB1");
        break;
    }
    case 2:
    {
        Serial.println("pB2");
        break;
    }
    case 3:
    {
        Serial.println("pBOver");
        break;
    }
    case 4:
    {
        Serial.println("nullptr");
        break;
    }
    default:
    {
        Serial.println("invalid buffer");
        break;
    }
    }
}

int getBufNum(struct printBuf *buf)
{
    if (buf == &pB1)
    {
        return 1;
    }
    else if (buf == &pB2)
    {
        return 2;
    }
    else if (buf == &pBOver)
    {
        return 3;
    }
    else if (buf == nullptr)
    {
        return 4;
    }
    else
    {
        return -1;
    }
}

void debugAll(String msg)
{
    Serial.println(msg);
    debug("stall print:     ", stall_print);
    debug("print ready:     ", print_ready_flag);
    debug("print overflow:  ", print_overflow_flag);
    debug("offset:          ", offset);
    debug("overflow offset: ", buf_overflow_offset);
    printBufInfo();
    Serial.println();
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

void serialEvent()
{
    char c = Serial.read();

    Serial.println(c);

    switch (c)
    {
    case 'c':
    {
        if (logger_state == IDLE)
        {
            logger_state = CREATE_FILE;
        }
        break;
    }
    case 's':
    {
        if (logger_state == FILE_LOADED)
        {
            logger_state = START_COLLECTION;
        }
        break;
    }
    case 'h':
    {
        if (logger_state == WRITE)
        {
            logger_state = CLOSE;
        }
        break;
    }
#ifdef SERIAL_DEBUG
    case 'd':
    {
        stall_print = !stall_print;
    }
#endif
    }
}