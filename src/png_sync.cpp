#include "png_sync.h"

void sync_init()
{
    pinMode(SYNC_OUT_PIN, OUTPUT);

#ifdef SYNC_CHAN
    pinMode(, INPUT);
#endif

    Serial.println(PSTR("Sync Initialized"));
}

