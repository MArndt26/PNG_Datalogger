#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>

#include <time.h>

/** Defines are mangaged by Makefile **/

// #define CREATE_INT_FILE
// #define CREATE_VOLTAGE_FILE

//Meta Data
// #define TRACK_EXTREME_DT

/** Run make help for execution instrutions **/

#define ADC_CHAN 10
#define MUXED_CHAN 3

typedef struct printLine
{
    unsigned int time;
    uint16_t data[ADC_CHAN * MUXED_CHAN];
    uint16_t sync;
} printLine;

enum output_t {
    INTEGER,
    VOLTAGE
};

FILE *inFilePtr;
FILE *outVoltFilePtr;
FILE *outIntFilePtr;

void createFiles(char * filename, char * fileTag, enum output_t outputType) {
    char *fileWithExtension = malloc(strlen(filename) + 11);

    sprintf(fileWithExtension, "%s.bin", filename);

    if ((inFilePtr = fopen(fileWithExtension, "rb")) == NULL)
    {
        printf("Error! Couldn't open file to read");

        // Program exits if the file pointer returns NULL.
        exit(1);
    }
    FILE *outFilePtr;

    switch (outputType) {
        case INTEGER: {
            sprintf(fileWithExtension, "%s(%s).csv", filename, fileTag);
            outIntFilePtr = fopen(fileWithExtension, "w");
            outFilePtr = outIntFilePtr;
            break;
        }
        case VOLTAGE: {
            sprintf(fileWithExtension, "%s(%s).csv", filename, fileTag);
            outVoltFilePtr = fopen(fileWithExtension, "w");
            outFilePtr = outVoltFilePtr;
            break;
        }
    }
   
    printf("...Created %s\n", fileWithExtension);

    int numCols = ADC_CHAN * MUXED_CHAN;

    for (int i = 0; i < numCols; i++)
    {
        if (i == 0)
        {
            fprintf(outFilePtr, "%10s, ", "time (us)");
            fprintf(outFilePtr, "%10s, ", "dt (us)");
            fprintf(outFilePtr, "%10s, ", "sync");
        }
        fprintf(outFilePtr, "%4d, ", i);
    }
    fprintf(outFilePtr, "\n"); //write header

    free(fileWithExtension);

    return;
}

void writeFiles(enum output_t outputType) {
    // struct printBuf pBuf;
    printLine pLine;

    FILE *outFilePtr;

    switch (outputType) {
        case INTEGER: {
            outFilePtr = outIntFilePtr;
            break;
        }
        case VOLTAGE: {
            outFilePtr = outVoltFilePtr;
            break;
        }
    }

#ifdef TRACK_EXTREME_DT
    int maxDT = INT_MIN;
    int minDT = INT_MAX;
#endif

    float max_val = 4096.0 - 1; //max value is max_val - 1
    float voltageRef = 3.3;

    int prevTime = 0;

    while (fread(&pLine, sizeof(printLine), 1, inFilePtr))
    {
        int curTime = pLine.time;

        int deltaT = curTime - prevTime;

        if (prevTime == 0)
        {
            deltaT = -999; //flagged as invalid data for first delta
        }
#ifdef TRACK_EXTREME_DT
        else //don't consider invalid data in meta
        {
            if (deltaT > maxDT)
            {
                maxDT = deltaT;
            }
            if (deltaT < minDT)
            {
                minDT = deltaT;
            }
        }
#endif
        fprintf(outFilePtr, "%10u, ", curTime);
        fprintf(outFilePtr, "%10d, ", deltaT);
        fprintf(outFilePtr, "%10d, ", pLine.sync);
        for (int j = 0; j < ADC_CHAN * MUXED_CHAN; j++)
        {
            switch (outputType) {
                case INTEGER : {
                    fprintf(outFilePtr, "%4d, ", pLine.data[j]);
                    break;
                }
                case VOLTAGE : {
                    float voltage = pLine.data[j] / max_val * voltageRef; //calculate voltage
                    fprintf(outFilePtr, "%4.6f, ", voltage);
                    break;
                }
            }
        }
        fprintf(outFilePtr, "\n");

        prevTime = pLine.time;
    }

#ifdef TRACK_EXTREME_DT
    printf("Max Delta T: %d\n", maxDT);
    printf("Min Delta T: %d\n", minDT);
#endif
    fclose(outFilePtr);
}

void *intThread(void *vargs) {
    char *filename = (char*) vargs;

    createFiles(filename, "int", INTEGER);

    writeFiles(INTEGER);

    printf("Finished Writing Integer File\n");

    return NULL;
}
void *voltThread(void *vargs) {
    char *filename = (char*) vargs;

    createFiles(filename, "volt", VOLTAGE);

    writeFiles(VOLTAGE);

    printf("Finished Writing Voltage File\n");

    return NULL;
}

int main(int argc, char *argv[])
{
    clock_t begin = clock();

#if defined(CREATE_VOLTAGE_FILE) || defined(CREATE_INT_FILE)
    if (argc != 2)
    {
        printf("Incorrect number of arguments!\n");
        exit(1);
    }
    else
    {
        printf("Starting Data Conversion...\n");
    }

    char *filename = argv[1];


#ifdef CREATE_VOLTAGE_FILE
    pthread_t vtid;
    pthread_create (&vtid, NULL, voltThread, filename);
    printf("Created Voltage Thread\n");
#endif

#ifdef CREATE_INT_FILE
    pthread_t itid;
    pthread_create (&itid, NULL, intThread, filename);
    printf("Created Integer Thread\n");
#endif

#ifdef CREATE_VOLTAGE_FILE
    pthread_join(vtid, NULL);
#endif
#ifdef CREATE_INT_FILE
    pthread_join(itid, NULL);
#endif

    fclose(inFilePtr);  //close inFile only

clock_t end = clock();
double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
printf("Finished in: %f sec\n", time_spent);

#else
    printf("*********************WARNING*************************\n");
    printf("***Must Define Output file in conversion.c defines***\n");
    printf("*****************************************************\n");
    exit(1);
#endif
}