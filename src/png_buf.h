#ifndef PNG_BUF_H
#define PNG_BUF_H

#include "main.h"
#include "png_adc.h"
#include "png_mux.h"

#define NUM_PRINT_LINES 7
#define CIRC_BUFF_SIZE 800

void buf_init();
void buf_clear();
bool writeReady();
uint8_t* write();
void nextwrite();
void fill_data(int i, uint16_t value);
bool next_line();
void fill_sync(uint16_t value);
void fill_time(unsigned int value);


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

#endif
