#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
 */

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Incorrect number of arguments!\n");
        exit(1);
    }

    char *filename = argv[1];
    int n;
    FILE *inFilePtr;
    FILE *outFilePtr;

    char *fileWithExtension = malloc(strlen(filename) + 4);

    sprintf(fileWithExtension, "%s.bin", filename);

    if ((inFilePtr = fopen(fileWithExtension, "rb")) == NULL)
    {
        printf("Error! Couldn't open file to read");

        // Program exits if the file pointer returns NULL.
        exit(1);
    }
    sprintf(fileWithExtension, "%s.csv", filename);

    outFilePtr = fopen(fileWithExtension, "w");

    uint16_t wBuff;
    uint32_t tBuff;

    int cols = 0;
    int numCols = 60;

    for (int i = 0; i < numCols; i++)
    {
        if (i == 0)
        {
            fprintf(outFilePtr, "%10s, ", "time (us)");
        }
        fprintf(outFilePtr, "%4d, ", i);
    }

    fprintf(outFilePtr, "\n");

    while (!feof(inFilePtr))
    {
        if (cols == 0)
        {
            fread(&tBuff, sizeof(uint32_t), 1, inFilePtr);
            fprintf(outFilePtr, "%10u, ", tBuff);
        }
        fread(&wBuff, sizeof(uint16_t), 1, inFilePtr);
        fprintf(outFilePtr, "%4d, ", wBuff);
        cols++;

        if (cols == numCols)
        {
            fprintf(outFilePtr, "\n");
            cols = 0;
        }
    }
    fclose(inFilePtr);
    fclose(outFilePtr);

    free(fileWithExtension);

    return 0;
}