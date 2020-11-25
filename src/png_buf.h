#ifndef PNG_BUF_H
#define PNG_BUF_H

#include "main.h"
#include "png_adc.h"
#include "png_mux.h"

#ifdef SERIAL_DEBUG
#define PRINT_BUF_MULT 5
#else
#define PRINT_BUF_MULT 2000
#endif

void buf_init();

struct printBuf
{
    unsigned int time[PRINT_BUF_MULT];
    uint16_t data[PRINT_BUF_MULT][ADC_CHAN * MUXED_CHAN];
    uint16_t sync[PRINT_BUF_MULT];
};

extern struct printBuf pB1;
extern struct printBuf pB2;
extern struct printBuf pBOver;

extern struct printBuf *wBuf;
extern struct printBuf *rBuf;

#define PRINTBUF_INIT(X) struct printBuf X = {0}

extern volatile int preOverflowBuffer;

extern volatile int offset;
extern volatile int buf_overflow_offset;

#endif
