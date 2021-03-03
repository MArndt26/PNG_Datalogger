#ifndef PNG_SD_H
#define PNG_SD_H

#include "main.h"
#include <SD.h>
#include <SPI.h>

#define FILE_BUF_SIZE 10

const int chipSelect = BUILTIN_SDCARD;

extern File dataFile;
extern char fName[FILE_BUF_SIZE];

void sd_init();
void sd_wrap_up();

#endif
