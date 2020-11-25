#ifndef PNG_SYNC_H
#define PNG_SYNC_H

#include "main.h"

#define SYNC_OUT_PIN 0

#ifdef SYNC_CHAN
    #define SYNC_IN_PIN A10
#endif

void sync_init();

#endif
