#include "png_buf.h"

int lineIdx = 0;  //make sure to init to 0
circBuf cBuf;

void buf_init()
{
    Serial.print("Buffer Initialized as [");
    Serial.print(CIRC_BUFF_SIZE);
    Serial.print("] print buffers with [");
    Serial.print(NUM_PRINT_LINES);
    Serial.print("] lines of size [");
    Serial.print(sizeof(printBuf));
    Serial.println("]");

    Serial.print("sizeof(printBuf) = ");
    Serial.println(sizeof(printBuf));
    Serial.print("sizeof(printLine) = ");
    Serial.println(sizeof(printLine));
    Serial.print("sizeof(circBuf) = ");
    Serial.println(sizeof(circBuf));


    buf_clear();
}

void buf_clear() 
{
    cBuf.rh = 0;
    cBuf.wh = 0;

    for (int i = 0; i < CIRC_BUFF_SIZE; i++)
    {
        cBuf.printReady[i] = 0;

        for (int j = 0; j < NUM_PRINT_LINES; j++)
        {
            cBuf.pb[i].line[j].time = 0;    //clear time to signal invalid data
        }
    }
}