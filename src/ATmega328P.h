#pragma once

#include <stdbool.h>
#include "AVR.h"
#include "common.h"

typedef struct ATmega238P_SRAM {
    byte reg[32];
    byte io_reg[64];
    byte ext_io_reg[160];
    byte data[2 * KB];
} ATmega238P_SRAM;

typedef struct ATmega238P_Memory {
    word flash[16 * KB];
    ATmega238P_SRAM sram;
    byte eeprom[1 * KB];
} ATmega238P_Memory;

// writes hex-records to flash
bool ATmega238P_WriteProgram(ATmega238P_Memory* memory, const char* hex);
