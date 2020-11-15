#ifndef PNG_MUX_H
#define PNG_MUX_H

#include <stdint.h>

const uint8_t mux_pins[] = {30, 31}; //A B

const uint8_t SEL_A = 0;
const uint8_t SEL_B = 1;

int mux_state = 0;

#define MUXED_CHAN 3

#endif
