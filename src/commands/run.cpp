#include "commands/run.h"
#include "png_buf.h"
#include "png_sd.h"

void run(int argc, char *argv[])
{
    printf("Initializing Buffer...\r\n");
    buf_clear(); //reset buffer

    char *filename;

    if (argc == 2)
    {
        filename = argv[1];
        dataFile = SD.open(filename, FILE_WRITE);
    }
    else
    {
        char finder[20] = "";

        int fNum = -1;
        do
        {
            //TODO: add timeout
            fNum++;
            sprintf(finder, "f%d.bin", fNum);
        } while (SD.exists(finder));

        filename = finder;

        printf("Using auto gen filename <%s>\r\n", filename);
        dataFile = SD.open(filename, FILE_WRITE);
    }

    if (!dataFile)
    {
        //TODO: add better error identification with FAT return val
        error(PSTR("Error with Datafile: Endless loop"));
    }
    printf("Writing to %s\r\n", filename);

    numWrites = 0; //reset number of writes

    printf("Running. Press any key to exit...\r\n");

#ifndef SERIAL_DEBUG
    adcTimer.begin(adc_isr, SAMPLING_PERIOD);
#else
    adcTimer.begin(adc_isr, SERIAL_DELAY);
#endif
    startTime = time;

    flush(); //flush serial

    printf("\r\n");

    //write loop
    while (!Serial.available())
    {
        if (writeReady())
        {
            digitalToggleFast(LED_BUILTIN);

            dataFile.write(write(), sizeof(printBuf));

            numWrites++;

            nextwrite();
        }
    }

    printf("num writes: %d\n", numWrites);

    // close out file
    adcTimer.end();

    flush(); //flush serial

    sd_wrap_up();

    printf("number of errors: %d\r\n", numErrors);
}
