#ifndef MAIN_H
#define MAIN_H

#include "Arduino.h"

#define SERIAL_DEBUG

extern volatile int print_ready_flag;
extern volatile int sd_print_comp_flag;
extern volatile int print_overflow_flag;

#define SAMPLING_PERIOD 500

extern int numWrites;
extern int numErrors;

extern elapsedMicros time;

extern unsigned int adcTime;

#endif
