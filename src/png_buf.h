#ifndef PNG_BUF_H
#define PNG_BUF_H

#include "main.h"
#include "png_adc.h"
#include "png_mux.h"

#ifdef SERIAL_DEBUG
#define PRINT_BUF_MULT 5
#else
#define PRINT_BUF_MULT 6000
#endif

void buf_init();
void buf_clear();

struct printLine
{
    unsigned int time;
    uint16_t data[ADC_CHAN * MUXED_CHAN];
    uint16_t sync;
};

extern struct printLine cBuf[PRINT_BUF_MULT];

extern volatile int cBufWriteIdx;
extern volatile int cBufReadIdx;

extern volatile int printIdle;

#endif
