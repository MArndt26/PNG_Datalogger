#ifndef MAIN_H
#define MAIN_H

#include "Arduino.h"

// uncomment for serial debugging mode at 1Hz
// #define SERIAL_DEBUG

// uncomment for extra adc channel to be used for sync signal input
#define SYNC_CHAN

extern volatile int print_ready_flag;
extern volatile int sd_print_comp_flag;
extern volatile int print_overflow_flag;

#define SAMPLING_PERIOD 500

extern int numWrites;
extern int numErrors;

extern elapsedMicros time;

extern unsigned int adcTime;

extern unsigned int startTime; //data collection start time

#define MAKE_ERRORS_FATAL

//useful shortcuts
#define printf Serial1.printf

#endif
