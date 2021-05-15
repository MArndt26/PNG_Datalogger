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
        } while (SD.exists(fName));

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

    printf("Running...\r\n");
}

// case CREATE_FILE:
// {

//     Serial.println(PSTR("Creating File..."));
//     int fNum = -1;
//     do
//     {
//         fNum++;
//         sprintf(fName, PSTR("F%d.bin"), fNum);
//     } while (SD.exists(fName));

//     Serial.print(PSTR("Filename Created: "));
//     Serial.println(fName);

//     dataFile = SD.open(fName, FILE_WRITE);

//     if (dataFile)
//     {
//         Serial.print(PSTR("File Loaded: "));
//         Serial.println(fName);
//         logger_state = FILE_LOADED;
//     }
//     else
//     {
//         error(PSTR("Error with Datafile: Endless loop"));
//     }
// #ifdef SERIAL_DEBUG
//     for (int i = 0; i < 9; i++)
//     {
//         Serial.print(' ');
//     }
//     for (int i = 0; i < 60; i++)
//     {
//         Serial.print(i);
//         Serial.print(',');
//     }
//     Serial.println();
// #endif

//     numWrites = 0; //reset number of writes

//     break;
// }
// case FILE_LOADED:
// {
//     blink(1, 500);
//     break;
// }
// case START_COLLECTION:
// {
// #ifndef SERIAL_DEBUG
//     adcTimer.begin(adc_isr, SAMPLING_PERIOD);
// #else
//     adcTimer.begin(adc_isr, SERIAL_DELAY);
// #endif
//     startTime = time;
//     logger_state = WRITE;
//     break;
// }
// case WRITE:
// {
//     if (writeReady())
//     {
//         digitalToggleFast(LED_BUILTIN);

//         dataFile.write(write(), sizeof(printBuf));

// #ifdef SERIAL_DEBUG
//         printCBuf(cBuf + cBufWriteIdx);
// #endif

//         numWrites++;

//         nextwrite();
//     }
//     break;
// }
// case CLOSE:
// {
//     adcTimer.end();

//     sd_wrap_up();

//     debug("number of errors: ", numErrors);

//     logger_state = IDLE;

//     break;
// }