#pragma once

#include "avr.h"
#include "common.h"

typedef struct sram {
    byte reg[32];
    byte io_reg[64];
    byte ext_io_reg[160];
    byte data[2 * KB];
} sram;

typedef struct mcu {
    word flash[16 * KB];
    sram sram;
    byte eeprom[1 * KB];
    cpu cpu;
} mcu;

// writes hex-records to flash
result program(mcu* mcu, const char* hex);

void cycle(mcu* mcu);
