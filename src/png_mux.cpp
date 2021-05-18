#include "png_mux.h"

const uint8_t mux_pins[] = {30, 31}; //A B

int mux_state = 0;

void mux_init()
{
    for (unsigned int i = 0; i < NUM_MUX_PINS; i++)
    {
        pinMode(mux_pins[i], OUTPUT);
        digitalWriteFast(mux_pins[i], LOW); //initialize to low
    }

    Serial1.print(PSTR("Mux Pins Initialized with ["));
    Serial1.print(MUXED_CHAN);
    Serial1.println(PSTR("] muxed channels"));
}