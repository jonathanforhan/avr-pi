#pragma once

#include <stdint.h>
#include "common.h"

#define OP_TYPE_JUMP 0b1001010000000000 // 0b1001010.........

#define OP_MASK_6    0b1111110000000000 // 0b111111..........
#define OP_MASK_7    0b1111111000000000 // 0b1111111.........
#define OP_MASK_JUMP 0b1111111000001110 // 0b1111111.....111.

#define OP_JMP 0b1001010000001100 // 1001010.....110.

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

typedef u8 byte;
typedef u16 word;
typedef u32 dword;
typedef u64 qword;

#define TO_WORD(HI, LO) ((((word)(HI)) << 8) | ((word)(LO)))

typedef struct cpu {
    u16 pc; // program counter

    u8 sreg;
    u8 work[32];

    u8 sph;
    u8 spl;
    u8 eearh;
    u8 eearl;
    u8 eedr;
    u8 eecr;
    u8 gpior2;
    u8 gpior1;
    u8 gpior0;
    u8 osccal;
    u8 clkpr;
    u8 smcr;
    u8 mcucr;
    u8 prr;
    u8 mcusr;
    u8 wdtcsr;
    u8 eicra;
    u8 eimsk;
    u8 eifr;
    u8 pcicr;
    u8 pcifr;
    u8 pcmsk2;
    u8 pcmsk1;
    u8 pcmsk0;
    u8 portb;
    u8 ddrb;
    u8 pinb;
    u8 portc;
    u8 ddrc;
    u8 pinc;
    u8 portd;
    u8 ddrd;
    u8 pind;
    u8 tccr0a;
    u8 tccr0b;
    u8 tcnt0;
    u8 ocr0a;
    u8 ocr0b;
    u8 timsk0;
    u8 tifr0;
    u8 tccr1a;
    u8 tccr1b;
    u8 tccr1c;
    u8 tcnt1h;
    u8 tcnt1l;
    u8 ocr1ah;
    u8 ocr1al;
    u8 ocr1bh;
    u8 ocr1bl;
    u8 icr1h;
    u8 icr1l;
    u8 timsk1;
    u8 tifr1;
    u8 gtccr;
    u8 tccr2a;
    u8 tccr2b;
    u8 tcnt2;
    u8 ocr2a;
    u8 ocr2b;
    u8 timsk2;
    u8 tifr2;
    u8 assr;
    u8 spcr;
    u8 spsr;
    u8 spdr;
    u8 udrn;
    u8 ucsrna;
    u8 ucsrnb;
    u8 ucsrnc;
    u8 ubrrnh;
    u8 ubrrnl;
    u8 twbr;
    u8 twcr;
    u8 twsr;
    u8 twdr;
    u8 twar;
    u8 twamr;
    u8 adcsrb;
    u8 acsr;
    u8 didr1;
    u8 admux;
    u8 adcsra;
    u8 adch;
    u8 adcl;
    u8 didr0;
    u8 dwdr;
    u8 spmcsr;
} cpu;
