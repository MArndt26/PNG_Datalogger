#ifndef MAIN_H
#define MAIN_H

#include "arduino.h"

volatile int print_ready_flag = 0;
volatile int sd_print_comp_flag = 1;
volatile int print_overflow_flag = 0;

#define SAMPLING_PERIOD 500

IntervalTimer adcTimer;

int numWrites = 0;
int numErrors = 0;

elapsedMicros time;

unsigned int adcTime;

#endif
