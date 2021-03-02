#ifndef PNG_BUF_H
#define PNG_BUF_H

#include "main.h"
#include "png_adc.h"
#include "png_mux.h"

// #ifdef SERIAL_DEBUG
// #define PRINT_BUF_MULT 5
// #else
// #endif

#define PRINT_BUF_MULT 2
#define PRINT_BUF_SIZE 10

void buf_init();
void buf_clear();
inline bool writeReady();
inline uint8_t* write();
inline void nextwrite();
inline void fill_data(int i, uint16_t value);
inline bool next_line();

typedef struct printLine
{
    unsigned int time;
    uint16_t data[ADC_CHAN * MUXED_CHAN];
    uint16_t sync;
} printLine;

typedef struct printBuf
{
    printLine line[PRINT_BUF_MULT];
} printBuf;

int lineIdx;  //make sure to init to 0

typedef struct circBuf
{
    printBuf pb[PRINT_BUF_SIZE];
    bool printReady[PRINT_BUF_SIZE];
    int wh; //write head (print)
    int rh; //read head (adc read)
} circBuf;

circBuf cBuf;

#endif
