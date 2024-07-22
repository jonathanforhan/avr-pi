#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "common.h"

typedef uint8_t byte;
typedef uint16_t word;

#define DATA_RECORD                  0
#define EOF_RECORD                   1
#define EXTENDED_SEGMENT_ADDR_RECORD 2
#define START_SEGMENT_ADDR_RECORD    3
#define EXTENDED_LINEAR_ADDR_RECORD  4
#define START_LINEAR_ADDR_RECORD     5

#define SREG_I 7
#define SREG_T 6
#define SREG_H 5
#define SREG_S 4
#define SREG_V 3
#define SREG_N 2
#define SREG_Z 1
#define SREG_C 0

#define REG_0  0x00
#define REG_1  0x01
#define REG_2  0x02
#define REG_3  0x03
#define REG_4  0x04
#define REG_5  0x05
#define REG_6  0x06
#define REG_7  0x07
#define REG_8  0x08
#define REG_9  0x09
#define REG_10 0x0a
#define REG_11 0x0b
#define REG_12 0x0c
#define REG_13 0x0d
#define REG_14 0x0e
#define REG_15 0x0f
#define REG_16 0x10
#define REG_17 0x11
#define REG_18 0x12
#define REG_19 0x13
#define REG_20 0x14
#define REG_21 0x15
#define REG_22 0x16
#define REG_23 0x17
#define REG_24 0x18
#define REG_25 0x19
#define REG_26 0x1a
#define REG_27 0x1b
#define REG_28 0x1c
#define REG_29 0x1d
#define REG_30 0x1e
#define REG_31 0x1f
