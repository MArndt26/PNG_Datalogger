#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CREATE_INT_FILE
#define CREATE_VOLTAGE_FILE

/**CONVERTS BINARY FILES TO CSV DATA
 * 
 * HOW TO:
 * - compile program into executable:
 *      gcc conversion.c
 * - run executable with command line args
 *      ./a.out <filename>
 *      - Note that filename doesn't include file extension
 *      - Example: filename => F0.BIN
 *          ./a.out F0
 * 
 *  UPDATE: use makefile for running tests
 *      Example Execution: make test args=F0
 * 
 */

#define ADC_CHAN 10
#define MUXED_CHAN 6
#define PRINT_BUF_MULT 3000

struct printBuf
{
    unsigned int time[PRINT_BUF_MULT];
    uint16_t data[PRINT_BUF_MULT][ADC_CHAN * MUXED_CHAN];
} printBuf;

int main(int argc, char *argv[])
{
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
    FILE *inFilePtr;

#ifdef CREATE_VOLTAGE_FILE
    FILE *outFilePtr;
#endif

#ifdef CREATE_INT_FILE
    FILE *intOutFilePtr;
#endif

    char *fileWithExtension = malloc(strlen(filename) + 10);

    sprintf(fileWithExtension, "%s.bin", filename);

    if ((inFilePtr = fopen(fileWithExtension, "rb")) == NULL)
    {
        printf("Error! Couldn't open file to read");

        // Program exits if the file pointer returns NULL.
        exit(1);
    }

#ifdef CREATE_VOLTAGE_FILE
    sprintf(fileWithExtension, "%s.csv", filename);
    outFilePtr = fopen(fileWithExtension, "w");
    printf("...Created %s\n", fileWithExtension);
#endif

#ifdef CREATE_INT_FILE
    sprintf(fileWithExtension, "%s(int).csv", filename);
    intOutFilePtr = fopen(fileWithExtension, "w");
    printf("...Created %s\n", fileWithExtension);
#endif

    int numCols = ADC_CHAN * MUXED_CHAN;
#ifdef CREATE_VOLTAGE_FILE
    float max_val = 4096.0 - 1; //max value is max_val - 1
    float voltageRef = 3.3;
#endif

    for (int i = 0; i < numCols; i++)
    {
        if (i == 0)
        {
#ifdef CREATE_VOLTAGE_FILE
            fprintf(outFilePtr, "%10s, ", "time (us)");
            fprintf(outFilePtr, "%10s, ", "dt (us)");
#endif
#ifdef CREATE_INT_FILE
            fprintf(intOutFilePtr, "%10s, ", "time (us)");
            fprintf(intOutFilePtr, "%10s, ", "dt (us)");
#endif
        }
#ifdef CREATE_VOLTAGE_FILE

        fprintf(outFilePtr, "%8d, ", i);
#endif
#ifdef CREATE_INT_FILE
        fprintf(intOutFilePtr, "%4d, ", i);
#endif
    }

#ifdef CREATE_VOLTAGE_FILE
    fprintf(outFilePtr, "\n");
#endif
#ifdef CREATE_INT_FILE
    fprintf(intOutFilePtr, "\n"); //write integer
#endif

    struct printBuf pBuf;

    int numDeletes = 0;
    int prevTime = 0;

    while (fread(&pBuf, sizeof(struct printBuf), 1, inFilePtr))
    {
        for (int i = 0; i < PRINT_BUF_MULT; i++)
        {
            int curTime = pBuf.time[i];

            if (curTime != 0)
            {

#ifdef CREATE_INT_FILE
                fprintf(intOutFilePtr, "%10u, ", curTime);
                fprintf(intOutFilePtr, "%10d, ", curTime - prevTime);
                for (int j = 0; j < ADC_CHAN * MUXED_CHAN; j++)
                {
                    fprintf(intOutFilePtr, "%4d, ", pBuf.data[i][j]);
                }
                fprintf(intOutFilePtr, "\n");
#endif

#ifdef CREATE_VOLTAGE_FILE
                fprintf(outFilePtr, "%10u, ", curTime);
                fprintf(outFilePtr, "%10d, ", curTime - prevTime);
                for (int j = 0; j < ADC_CHAN * MUXED_CHAN; j++)
                {
                    float voltage = pBuf.data[i][j] / max_val * voltageRef; //calculate voltage
                    fprintf(outFilePtr, "%4.6f, ", voltage);
                }
                fprintf(outFilePtr, "\n");
#endif

                prevTime = pBuf.time[i];
            }
            else
            {
                numDeletes++;
            }
        }
    }

    printf("Number of rows deleted: %d\n", numDeletes);

    fclose(inFilePtr);

#ifdef CREATE_VOLTAGE_FILE
    fclose(outFilePtr);
#endif

#ifdef CREATE_INT_FILE
    fclose(intOutFilePtr);
#endif

    free(fileWithExtension);

    printf("Finished writing files\n");

    return 0;

#else
    printf("*********************WARNING*************************\n");
    printf("***Must Define Output file in conversion.c defines***\n");
    printf("*****************************************************\n");
    exit(1);
#endif
}