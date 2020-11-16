#include "png_buf.h"

// PRINTBUF_INIT(X);

struct printBuf pB1;
struct printBuf pB2;
struct printBuf pBOver;

struct printBuf *wBuf = nullptr;
struct printBuf *rBuf = &pB1;

volatile int preOverflowBuffer = 0;

volatile int offset;
volatile int buf_overflow_offset = 0;

void buf_init()
{
    Serial.print("Buffer Initialized with size [");
    Serial.print(PRINT_BUF_MULT);
    Serial.println("]");
}