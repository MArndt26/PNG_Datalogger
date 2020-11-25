#ifndef PNG_SYNC_H
#define PNG_SYNC_H

#include "main.h"

#define SYNC_OUT_PIN 0

#ifdef SYNC_CHAN
    #define SYNC_IN_PIN A11
#endif

void sync_init();

extern volatile int sync_count;
extern volatile int sync_stage_count;

extern const int sync_stage_count_switch;  
extern const int sync_pulse_period; 
extern const int sync_wait_period ; 


#endif
