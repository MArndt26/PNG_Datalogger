#ifndef _SERIAL_H
#define _SERIAL_H

void serialEvent();

enum SERIAL_CMD
{
    START = 's',
    HALT = 'h',
    CREATE_FILE = 'c',
};

#endif /* _SERIAL_H  */