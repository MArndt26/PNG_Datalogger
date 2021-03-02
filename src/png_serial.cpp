#include "main.h"
#include "png_serial.h"
#include "png_buf.h"
#include "png_adc.h"
#include "png_mux.h"
#include "png_states.h"
#include "png_sync.h"

int stall_print = 0;

void printCBuf(struct printLine *buf)
{
    Serial.print(buf->time);

    int d = countDigits(buf->time);

    while (d < 15)
    {
        Serial.print(PSTR(" "));
        d++;
    }

    Serial.print(',');
    for (int i = 0; i < ADC_CHAN; i++)
    {
        Serial.print(buf->data[i]);

        Serial.print(',');
    }
    Serial.println();
}

void error(const char *msg)
{
    adcTimer.end();

    Serial.println(msg);

    debug("numWrites: ", numWrites);

    debug("Write Offset: ", cBuf.wh);

    debug("Read Offset: ", cBuf.rh);

    while (1)
    {
        blink(1, 200);
    }
}

void debugAll(const char *msg)
{
    Serial.println(msg);
    debugFormat("stall print:", stall_print);
    debugFormat("print ready:", print_ready_flag);
    debugFormat("print overflow:", print_overflow_flag);
    debugFormat("Write Offset: ", cBuf.wh);
    debugFormat("Read Offset: ", cBuf.rh);
    debugFormat("sync count:", sync_count);
    debugFormat("sync stage count:", sync_stage_count);
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
    Serial.print("Build Date/Time: ");
    Serial.println(PSTR(__TIMESTAMP__));
    Serial.println(PSTR("Serial Connected"));
}