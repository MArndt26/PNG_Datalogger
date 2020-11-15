#ifndef PNG_SERIAL_H
#define PNG_SERIAL_H

#include "main.h"

#define SERIAL_DELAY 1000000

extern int stall_print;

#ifdef SERIAL_DEBUG
const int SERIAL_BUF_DISP = 5;
#else
const int SERIAL_BUF_DISP = 5;
#endif

void debugAll(const char *msg);

void debugFormat(const char *msg, int val);

void printBufInfo();

void printPBuf(int offset, struct printBuf *buf);

void printBuffer(const char *name, struct printBuf *buf);

int getBufNum(struct printBuf *buf);

void error(const char *msg);

int countDigits(int n);

void blink(int times, int d);

void debug(const char *msg, int val);

void serial_init();

#endif
