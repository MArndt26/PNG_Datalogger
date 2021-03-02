#include "png_buf.h"

struct printBuf cBuf[PRINT_BUF_SIZE];

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
    cBuf.rh = 0;
    cBuf.wh = 0;

    for (int i = 0; i < PRINT_BUF_SIZE; i++)
    {
        cBuf.printReady[i] = 0;

        for (int j = 0; j < PRINT_BUF_MULT; j++)
        {
            cBuf.pb[i].line[j].time = 0;    //clear time to signal invalid data
        }
    }
}

inline bool writeReady()
{
    return cBuf.printReady[cBuf.wh];
}

inline uint8_t* write()
{
    return (uint8_t*) &cBuf.pb[cBuf.wh];
}

inline void nextwrite()
{
    cBuf.printReady[cBuf.wh] = 0;
    cBuf.wh++;
    if (cBuf.wh >= PRINT_BUF_SIZE)
    {
        //overflow case
        cBuf.wh = 0;
    }
}

inline void fill_data(int i, uint16_t value)
{
    //read adc value to data at index i
    cBuf.pb[cBuf.rh].line[lineIdx].data[i] = value;
}

inline bool next_line() 
{
    //increment print line index
    lineIdx++;   
    if (lineIdx >= PRINT_BUF_MULT)
    {
        //reset line index
        lineIdx = 0;
        // set line pb print ready
        cBuf.printReady[cBuf.rh];
        // increment read head
        cBuf.rh++;

        if (cBuf.rh >= PRINT_BUF_SIZE)
        {
            //overflow case
            cBuf.rh = 0;
        }

        if (cBuf.rh == cBuf.wh) {
            // overwrite case
            // decrement rh to stall
            cBuf.rh--;
            if (cBuf.rh < 0) 
            {
                //overwrite occured at 0 corner case
                cBuf.rh = PRINT_BUF_SIZE - 1;
            }
            return false;
        }
    }
    return true;
}