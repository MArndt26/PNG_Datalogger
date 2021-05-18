#include "png_buf.h"

int lineIdx = 0; //make sure to init to 0
circBuf cBuf;

void buf_init()
{
    printf("Buffer Initialized\r\n");
    printf("Circular Buffer <%d> bytes\r\n", sizeof(circBuf));
    printf("|-- Print Buffer <%d> bytes (x%d)\r\n", sizeof(printBuf), CIRC_BUFF_SIZE);
    printf("|   |-- Print Line <%d> bytes (x%d)\r\n", sizeof(printLine), NUM_PRINT_LINES);

    buf_clear();
}

void buf_clear()
{
    // reset read/write heads
    cBuf.rh = 0;
    cBuf.wh = 0;

    // reset tracking stats
    numWrites = 0;
    numErrors = 0;

    for (int i = 0; i < CIRC_BUFF_SIZE; i++)
    {
        cBuf.printReady[i] = 0;

        for (int j = 0; j < NUM_PRINT_LINES; j++)
        {
            cBuf.pb[i].line[j].time = 0; //clear time to signal invalid data
        }
    }
}