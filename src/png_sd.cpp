#include "png_sd.h"

File dataFile;
char fName[FILE_BUF_SIZE];

void sd_init()
{
    Serial.print(PSTR("Initializing SD card..."));

    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect))
    {
        Serial.println(PSTR("Card failed, or not present"));
        // don't do anything more:
        return;
    }

    Serial.println(PSTR("card initialized."));
}