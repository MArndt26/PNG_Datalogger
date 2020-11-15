#ifndef PNG_STATES_H
#define PNG_STATES_H

#include "main.h"

enum LOGGER_STATE
{
    IDLE,
    CREATE_FILE,
    FILE_LOADED,
    START_COLLECTION,
    WRITE,
    CLOSE
};

extern LOGGER_STATE logger_state;

#endif
