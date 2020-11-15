#ifndef PNG_BUF_H
#define PNG_BUF_H

#include <stdint.h>
#include "png_adc.h"
#include "png_mux.h"

#ifdef SERIAL_DEBUG
const int PRINT_BUF_MULT = 5;
#else
const int PRINT_BUF_MULT = 2000;
#endif

struct printBuf
{
    unsigned int time[PRINT_BUF_MULT];
    uint16_t data[PRINT_BUF_MULT][ADC_CHAN * MUXED_CHAN];
};

struct printBuf pB1;
struct printBuf pB2;
struct printBuf pBOver;

struct printBuf *wBuf = nullptr;
struct printBuf *rBuf = &pB1;

volatile int preOverflowBuffer = 0;

volatile int offset;
volatile int buf_overflow_offset = 0;

#endif
