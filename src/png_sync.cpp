#include "png_sync.h"

void sync_init()
{
    pinMode(SYNC_PIN, OUTPUT);

    Serial.println(PSTR("Sync Initialized"));
}