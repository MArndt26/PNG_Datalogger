#include "png_sd.h"
#include "png_serial.h"
#include "png_buf.h"

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

void sd_wrap_up()
{
    unsigned int stopTime = time;

    Serial.println(PSTR("Halted Data Collection"));
    Serial.println(PSTR("wrapping up file..."));

    Serial.println(PSTR("Data Collection Time: "));
    Serial.print(stopTime - startTime);
    Serial.print(" micro or ");
    Serial.print((stopTime - startTime) / 1e6);
    Serial.println(" sec");

    Serial.println("Writing Remaining Buffer...");

    while (writeReady())
    {
        dataFile.write(write(), sizeof(printBuf));

        numWrites++;

        nextwrite();
    }

	dataFile.close();

    Serial.println("Buffer Flushed");
}