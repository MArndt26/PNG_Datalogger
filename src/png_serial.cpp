#include "main.h"
#include "png_serial.h"
#include "png_buf.h"
#include "png_adc.h"
#include "png_mux.h"
#include "png_states.h"
#include "png_sync.h"

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

void error(const char *msg)
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

void printBuffer(const char *name, struct printBuf *buf)
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

void debugAll(const char *msg)
{
    Serial.println(msg);
    debugFormat("stall print:", stall_print);
    debugFormat("print ready:", print_ready_flag);
    debugFormat("print overflow:", print_overflow_flag);
    debugFormat("offset:", offset);
    debugFormat("overflow offset:", buf_overflow_offset);
    debugFormat("sync count:", sync_count);
    debugFormat("sync stage count:", sync_stage_count);
    printBufInfo();
    Serial.println();
}

void debugFormat(const char *msg, int val)
{
    const int bufSize = 50;
    char buf[bufSize];
    sprintf(buf, "|%-20s|%-5d|\n", msg, val);
    Serial.print(buf);
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

void debug(const char *msg, int val)
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

void serial_init()
{
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    while (!Serial)
        ; // wait for serial port to connect.
    Serial.println(PSTR("Serial Connected"));
}