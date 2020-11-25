#include "png_sync.h"

void sync_init()
{
    pinMode(SYNC_OUT_PIN, OUTPUT);

#ifdef SYNC_CHAN
    pinMode(SYNC_IN_PIN, INPUT);
#endif

    Serial.println(PSTR("Sync Initialized"));
}

