#ifndef PNG_SERIAL_H
#define PNG_SERIAL_H

#include "Arduino.h"

// #define SERIAL_DEBUG
#define SERIAL_DELAY 1000000
int stall_print = 0;

#ifdef SERIAL_DEBUG
const int SERIAL_BUF_DISP = 5;
#else
const int SERIAL_BUF_DISP = 5;
#endif

void debugAll(String msg);

void printBufInfo();

void printPBuf(int offset, struct printBuf *buf);

void printBuffer(String name, struct printBuf *buf);

int getBufNum(struct printBuf *buf);

void error(const String msg);

int countDigits(int n);

void blink(int times, int d);

void debug(String msg, int val);

#endif
