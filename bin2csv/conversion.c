#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CREATE_INT_FILE

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

#define ADC_CHAN 15
#define PRINT_BUF_MULT 10000

struct printBuf
{
    unsigned int time[PRINT_BUF_MULT];
    uint16_t data[PRINT_BUF_MULT][ADC_CHAN];
} printBuf;

int main(int argc, char *argv[])
{
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
    FILE *outFilePtr;

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

    sprintf(fileWithExtension, "%s.csv", filename);
    outFilePtr = fopen(fileWithExtension, "w");
    printf("...Created %s\n", fileWithExtension);

#ifdef CREATE_INT_FILE
    sprintf(fileWithExtension, "%s(int).csv", filename);
    intOutFilePtr = fopen(fileWithExtension, "w");
    printf("...Created %s\n", fileWithExtension);
#endif

    int cols = 0;
    int numCols = ADC_CHAN;
    float max_val = 4096.0 - 1; //max value is max_val - 1
    float voltageRef = 3.3;

    for (int i = 0; i < numCols; i++)
    {
        if (i == 0)
        {
            fprintf(outFilePtr, "%10s, ", "time (us)");
#ifdef CREATE_INT_FILE
            fprintf(intOutFilePtr, "%10s, ", "time (us)");
#endif
        }
        fprintf(outFilePtr, "%8d, ", i);
#ifdef CREATE_INT_FILE
        fprintf(intOutFilePtr, "%4d, ", i);
#endif
    }

    fprintf(outFilePtr, "\n");
#ifdef CREATE_INT_FILE
    fprintf(intOutFilePtr, "\n"); //write integer
#endif

    struct printBuf pBuf;

    while (fread(&pBuf, sizeof(struct printBuf), 1, inFilePtr))
    {
        for (int i = 0; i < PRINT_BUF_MULT; i++)
        {
#ifdef CREATE_INT_FILE
            fprintf(intOutFilePtr, "%10u, ", pBuf.time[i]);
            for (int j = 0; j < ADC_CHAN; j++)
            {
                fprintf(intOutFilePtr, "%4d, ", pBuf.data[i][j]);
            }
            fprintf(intOutFilePtr, "\n");
#endif
            fprintf(outFilePtr, "%10u, ", pBuf.time[i]);
            for (int j = 0; j < ADC_CHAN; j++)
            {
                float voltage = pBuf.data[i][j] / max_val * voltageRef; //calculate voltage
                fprintf(outFilePtr, "%4.6f, ", voltage);
            }
            fprintf(outFilePtr, "\n");

            cols++;
        }
    }

    fclose(inFilePtr);
    fclose(outFilePtr);

#ifdef CREATE_INT_FILE
    fclose(intOutFilePtr);
#endif

    free(fileWithExtension);

    printf("Finished writing files\n");

    return 0;
}