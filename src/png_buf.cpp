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

bool writeReady()
{
    return cBuf.printReady[cBuf.wh];
}

uint8_t* write()
{
    return (uint8_t*) &cBuf.pb[cBuf.wh];
}

void nextwrite()
{
    cBuf.printReady[cBuf.wh] = 0;
    cBuf.wh++;
    if (cBuf.wh >= NUM_PRINT_LINES)
    {
        //overflow case
        cBuf.wh = 0;
    }
}

void fill_data(int i, uint16_t value)
{
    //read adc value to data at index i
    cBuf.pb[cBuf.rh].line[lineIdx].data[i] = value;
}

void fill_sync(uint16_t value)
{
    //read adc value to sync
    cBuf.pb[cBuf.rh].line[lineIdx].sync = value;
}

void fill_time(unsigned int value)
{
    //read adc value to time
    cBuf.pb[cBuf.rh].line[lineIdx].time = value;
}

bool next_line() 
{
    //increment print line index
    lineIdx++;   
    if (lineIdx >= NUM_PRINT_LINES)
    {
        //reset line index
        lineIdx = 0;
        // set line pb print ready
        cBuf.printReady[cBuf.rh] = 1;
        // increment read head
        cBuf.rh++;

        if (cBuf.rh >= CIRC_BUFF_SIZE)
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
                cBuf.rh = CIRC_BUFF_SIZE - 1;
            }
            return false;
        }
    }
    return true;
}