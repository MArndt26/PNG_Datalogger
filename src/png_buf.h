#ifndef PNG_BUF_H
#define PNG_BUF_H

#include "main.h"
#include "png_adc.h"
#include "png_mux.h"
#include "png_serial.h"

#define NUM_PRINT_LINES 15
#define CIRC_BUFF_SIZE 400

typedef struct printLine
{
    unsigned int time;
    uint16_t data[ADC_CHAN * MUXED_CHAN];
    uint16_t sync;
} printLine;

typedef struct printBuf
{
    printLine line[NUM_PRINT_LINES];
} printBuf;

extern int lineIdx;  //make sure to init to 0

typedef struct circBuf
{
    printBuf pb[CIRC_BUFF_SIZE];
    bool printReady[CIRC_BUFF_SIZE];
    int wh; //write head (print)
    int rh; //read head (adc read)
} circBuf;

extern circBuf cBuf;

// Function Headers
void buf_init();
void buf_clear();
uint8_t* write();

// Inline functions
inline bool writeReady()
{
    return cBuf.printReady[cBuf.wh];
}

inline void nextwrite()
{
    cBuf.printReady[cBuf.wh] = 0;
    cBuf.wh++;
    if (cBuf.wh >= CIRC_BUFF_SIZE)
    {
        //overflow case
        cBuf.wh = 0;
    }
}

inline uint8_t* write()
{
    return (uint8_t*) &cBuf.pb[cBuf.wh];
}

inline void fill_data(int i, uint16_t value)
{
    //read adc value to data at index i
    cBuf.pb[cBuf.rh].line[lineIdx].data[i] = value;
}

inline void fill_sync(uint16_t value)
{
    //read adc value to sync
    cBuf.pb[cBuf.rh].line[lineIdx].sync = value;
}

inline void fill_time(unsigned int value)
{
    //read adc value to time
    cBuf.pb[cBuf.rh].line[lineIdx].time = value;
}

inline bool bufferOverun()
{
    return (cBuf.rh == cBuf.wh && cBuf.printReady[cBuf.wh]);
}

inline void next_line() 
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

        if (cBuf.wh == cBuf.rh) 
        {
            error("cBuf overwrite");
        }
    }
}

#endif
