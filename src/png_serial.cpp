#include "main.h"
#include "png_serial.h"
#include "png_buf.h"
#include "png_adc.h"
#include "png_mux.h"
#include "png_states.h"
#include "png_sync.h"
#include "png_sd.h"

int user_interrupt_flag = 0;
int stall_print = 0;

void printCBuf(struct printLine *buf)
{
    Serial1.print(buf->time);

    int d = countDigits(buf->time);

    while (d < 15)
    {
        Serial1.print(PSTR(" "));
        d++;
    }

    Serial1.print(',');
    for (int i = 0; i < ADC_CHAN; i++)
    {
        Serial1.print(buf->data[i]);

        Serial1.print(',');
    }
    Serial1.println();
}

void error(const char *msg)
{
    adcTimer.end();

    Serial1.println(msg);

    debug("numWrites: ", numWrites);

    debug("Write Offset: ", cBuf.wh);

    debug("Read Offset: ", cBuf.rh);

    sd_wrap_up();

    Serial1.println("...Entering Endless Loop...");

    while (1)
    {
        blink(1, 200);
    }
}

void debugAll(const char *msg)
{
    Serial1.println(msg);
    debugFormat("stall print:", stall_print);
    debugFormat("print ready:", print_ready_flag);
    debugFormat("print overflow:", print_overflow_flag);
    debugFormat("Write Offset: ", cBuf.wh);
    debugFormat("Read Offset: ", cBuf.rh);
    debugFormat("sync count:", sync_count);
    debugFormat("sync stage count:", sync_stage_count);
    Serial1.println();
}

void debugFormat(const char *msg, int val)
{
    const int bufSize = 50;
    char buf[bufSize];
    sprintf(buf, "|%-20s|%-5d|\n", msg, val);
    Serial1.print(buf);
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
    Serial1.print(msg);
    Serial1.println(val);
}

//TODO: user serialEvent in same way as serialISR
// void serialEvent()
// {
//     char c = Serial1.read();

//     Serial1.println(c);
//     Serial1.println("wtf is the code doing here");

//     user_interrupt_flag = 1;

//     switch (c)
//     {
//     case 'c':
//     {
//         if (logger_state == IDLE)
//         {
//             logger_state = CREATE_FILE;
//         }
//         break;
//     }
//     case 's':
//     {
//         if (logger_state == FILE_LOADED)
//         {
//             logger_state = START_COLLECTION;
//         }
//         break;
//     }
//     case 'h':
//     {
//         if (logger_state == WRITE)
//         {
//             logger_state = CLOSE;
//         }
//         break;
//     }
// #ifdef SERIAL_DEBUG
//     case 'd':
//     {
//         stall_print = !stall_print;
//     }
// #endif
//     }
// }

void flush()
{
    while (Serial1.available())
    {
        //flush serial buffer
        Serial1.read();
    };
}

void serial_init()
{
    // Open serial communications and wait for port to open:
    Serial1.begin(115200);
    while (!Serial1)
    {
        // wait for serial port to connect.
        blink(2, 100);
    }

    printf("Build Date/Time: %s\r\n", __TIMESTAMP__);
    printf("Serial Connected\r\n");
}