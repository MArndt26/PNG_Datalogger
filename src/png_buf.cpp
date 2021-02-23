#include "png_buf.h"

struct printLine cBuf[PRINT_BUF_MULT];

volatile int cBufWriteIdx;
volatile int cBufReadIdx;

volatile int printIdle;


void buf_init()
{
    Serial.print("Buffer Initialized with size [");
    Serial.print(PRINT_BUF_MULT);
    Serial.println("]");

    buf_clear();
}

void buf_clear() 
{
    cBufWriteIdx = -1;   //reset write index
    cBufReadIdx = 0;    //reset read index
    printIdle = 1;      //reset print idlel

    for (int i = 0; i < PRINT_BUF_MULT; i++)
    {
        cBuf[i].time = 0; //clear time to signal invalid data
    }
}