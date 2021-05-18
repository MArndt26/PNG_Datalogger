#include "png_sync.h"

volatile int sync_count = 0;
volatile int sync_stage_count = 0;

const int sync_stage_count_switch = 8; // 8 toggles -> 4 pulses
const int sync_pulse_period = 250;     //2000 / 250 = 8 toggles -> 4 pulses in 1 second
const int sync_wait_period = 2000 * 4; //wait 4 seconds

// sync wave is 4 pules in 1 second (square wave 50% duty cycle)
//  then wait 4 seconds,
//  repeat

void sync_init()
{
    pinMode(SYNC_OUT_PIN, OUTPUT);
    digitalWriteFast(SYNC_OUT_PIN, LOW);

#ifdef SYNC_CHAN
    pinMode(SYNC_IN_PIN, INPUT);
    Serial1.println(PSTR("Sync_In Initialized"));
#endif

    Serial1.println(PSTR("Sync Initialized"));
}
