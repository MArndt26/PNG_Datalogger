#include "png_sd.h"
#include "png_serial.h"
#include "png_buf.h"

File dataFile;
char fName[FILE_BUF_SIZE];

void sd_init()
{
    Serial1.print(PSTR("Initializing SD card..."));

    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect))
    {
        Serial1.println(PSTR("Card failed, or not present"));
        // don't do anything more:
        return;
    }

    Serial1.println(PSTR("card initialized."));
}

void sd_wrap_up()
{
    unsigned int stopTime = time;

    Serial1.println(PSTR("Halted Data Collection"));
    Serial1.println(PSTR("wrapping up file..."));

    Serial1.println(PSTR("Data Collection Time: "));
    Serial1.print(stopTime - startTime);
    Serial1.print(" micro or ");
    Serial1.print((stopTime - startTime) / 1e6);
    Serial1.println(" sec");

    Serial1.println("Writing Remaining Buffer...");

    while (writeReady())
    {
        dataFile.write(write(), sizeof(printBuf));

        numWrites++;

        nextwrite();
    }

    dataFile.close();

    Serial1.println("Buffer Flushed");
}