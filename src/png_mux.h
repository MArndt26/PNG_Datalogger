#ifndef PNG_MUX_H
#define PNG_MUX_H

#include "main.h"

#define NUM_MUX_PINS 2

#define MUXED_CHAN 3

#define SEL_A 0
#define SEL_B 1

extern const uint8_t mux_pins[];

extern int mux_state;

#endif
