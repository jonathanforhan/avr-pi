/**
 * avr-pi
 * Copyright (C) 2024 Jonathan Forhan <jonathan.forhan@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _AVR__AVR_DEFS_H_
#define _AVR__AVR_DEFS_H_

/*******************************************************************************
 * Hex Record Type
 ******************************************************************************/
#define DATA_RECORD                  0
#define EOF_RECORD                   1
#define EXTENDED_SEGMENT_ADDR_RECORD 2
#define START_SEGMENT_ADDR_RECORD    3
#define EXTENDED_LINEAR_ADDR_RECORD  4
#define START_LINEAR_ADDR_RECORD     5

/*******************************************************************************
 * Status Register (SREG)
 *
 * C : Carry Flag
 * Z : Zero Flag
 * N : Negative Flag
 * V : Two's complement overflow indicator
 * S : N ^ V, for signed tests
 * H : Hald Carry Flag
 * T : Transfer bit used by BLD and BST instructions
 * I : Global Interrupt Enable/Disable Flag
 ******************************************************************************/
#define SREG_C 0
#define SREG_Z 1
#define SREG_N 2
#define SREG_V 3
#define SREG_S 4
#define SREG_H 5
#define SREG_T 6
#define SREG_I 7

/*******************************************************************************
 * Registers and Operands
 * UNUSED: Instruction is a duplicate and the parent is called instead
 *
 * Rd: Destination (and source) register in the Register File
 * Rr: Source register in the Register File
 * R: Result after instruction is executed
 * K: Constant data
 * k: Constant address
 * b: Bit in the Register File or I/O Register (3-bit)
 * s: Bit in the Status Register (3-bit)
 * X,Y,Z: Indirect Address Register (X=R27:R26, Y=R29:R28, and Z=R31:R30)
 * A: I/O location address
 * q: Displacement for direct addressing (6-bit)
 ******************************************************************************/
#define REG_X 26 // R26:R27
#define REG_Y 28 // R28:R29
#define REG_Z 30 // R30:R31

/* Arithmetic and Logic Instructions */
#define OP_ADD          0x0C00 //  [6]  0000 11rd dddd rrrr
#define OP_ADC          0x1C00 //  [6]  0001 11rd dddd rrrr
#define OP_ADIW         0x9600 //  [8]  1001 0110 KKdd KKKK
#define OP_SUB          0x1800 //  [6]  0001 10rd dddd rrrr
#define OP_SUBI         0x5000 //  [4]  0101 KKKK dddd KKKK
#define OP_SBC          0x0800 //  [6]  0000 10rd dddd rrrr
#define OP_SBCI         0x4000 //  [4]  0100 KKKK dddd KKKK
#define OP_SBIW         0x9700 //  [6]  1001 0111 KKdd KKKK
#define OP_AND          0x2000 //  [6]  0010 00rd dddd rrrr
#define OP_ANDI         0x7000 //  [4]  0111 KKKK dddd KKKK
#define OP_OR           0x2800 //  [6]  0010 10rd dddd rrrr
#define OP_ORI          0x6000 //  [4]  0110 KKKK dddd KKKK
#define OP_EOR          0x2400 //  [6]  0010 01rd dddd rrrr
#define OP_COM          0x9400 //  [9]  1001 010d dddd 0000
#define OP_NEG          0x9401 //  [7]  1001 010d dddd 0001
#define OP_SBR          0x6000 //  [4]  0110 KKKK dddd KKKK  (UNUSED)
#define OP_CBR          0x7000 //  [4]  0111 KKKK dddd KKKK  (UNUSED)
#define OP_INC          0x9403 //  [7]  1001 010d dddd 0011
#define OP_DEC          0x940A //  [7]  1001 010d dddd 1010
#define OP_TST          0x2000 //  [6]  0010 00dd dddd dddd  (UNUSED)
#define OP_CLR          0x2400 //  [6]  0010 01dd dddd dddd  (UNUSED)
#define OP_SER          0xEF0F //  [8]  1110 1111 dddd 1111
#define OP_MUL          0x9C00 //  [6]  1001 11rd dddd rrrr
#define OP_MULS         0x0200 //  [8]  0000 0010 dddd rrrr
#define OP_MULSU        0x0300 //  [9]  0000 0011 0ddd 0rrr
#define OP_FMUL         0x0308 //  [9]  0000 0011 0ddd 1rrr
#define OP_FMULS        0x0380 //  [9]  0000 0011 1ddd 0rrr
#define OP_FMULSU       0x0388 //  [9]  0000 0011 1ddd 1rrr
/* Branch Instructions */
#define OP_RJMP         0xC000 //  [4]  1100 kkkk kkkk kkkk
#define OP_IJMP         0x9409 // [16]  1001 0100 0000 1001
#define OP_JMP          0x940C //  [7]  1001 010k kkkk 110k  kkkk kkkk kkkk kkkk
#define OP_RCALL        0xD000 //  [4]  1101 kkkk kkkk kkkk
#define OP_ICALL        0x9509 // [16]  1001 0101 0000 1001
#define OP_CALL         0x940E //  [7]  1001 010k kkkk 111k  kkkk kkkk kkkk kkkk
#define OP_RET          0x9508 // [16]  1001 0101 0000 1000
#define OP_RETI         0x9518 // [16]  1001 0101 0001 1000
#define OP_CPSE         0x1000 //  [6]  0001 00rd dddd rrrr
#define OP_CP           0x1400 //  [6]  0001 01rd dddd rrrr
#define OP_CPC          0x0400 //  [6]  0000 01rd dddd rrrr
#define OP_CPI          0x3000 //  [4]  0011 KKKK dddd KKKK
#define OP_SBRC         0xFC00 //  [7]  1111 110r rrrr 0bbb
#define OP_SBRS         0xFE00 //  [7]  1111 111r rrrr 0bbb
#define OP_SBIC         0x9900 //  [8]  1001 1001 AAAA Abbb
#define OP_SBIS         0x9B00 //  [8]  1001 1011 AAAA Abbb
#define OP_BRBS         0xF000 //  [6]  1111 00kk kkkk ksss
#define OP_BRBC         0xF400 //  [6]  1111 01kk kkkk ksss
#define OP_BREQ         0xF001 //  [6]  1111 00kk kkkk k001  (UNUSED)
#define OP_BRNE         0xF401 //  [6]  1111 01kk kkkk k001  (UNUSED)
#define OP_BRCS         0xF000 //  [6]  1111 00kk kkkk k000  (UNUSED)
#define OP_BRCC         0xF400 //  [6]  1111 01kk kkkk k000  (UNUSED)
#define OP_BRSH         0xF400 //  [6]  1111 01kk kkkk k000  (UNUSED)
#define OP_BRLO         0xF000 //  [6]  1111 00kk kkkk k000  (UNUSED)
#define OP_BRMI         0xF002 //  [6]  1111 00kk kkkk k010  (UNUSED)
#define OP_BRPL         0xF402 //  [6]  1111 01kk kkkk k010  (UNUSED)
#define OP_BRGE         0xF404 //  [6]  1111 01kk kkkk k100  (UNUSED)
#define OP_BRLT         0xF004 //  [6]  1111 00kk kkkk k100  (UNUSED)
#define OP_BRHS         0xF005 //  [6]  1111 00kk kkkk k101  (UNUSED)
#define OP_BRHC         0xF405 //  [6]  1111 01kk kkkk k101  (UNUSED)
#define OP_BRTS         0xF006 //  [6]  1111 00kk kkkk k110  (UNUSED)
#define OP_BRTC         0xF406 //  [6]  1111 01kk kkkk k110  (UNUSED)
#define OP_BRVS         0xF003 //  [6]  1111 00kk kkkk k011  (UNUSED)
#define OP_BRVC         0xF403 //  [6]  1111 01kk kkkk k011  (UNUSED)
#define OP_BRIE         0xF007 //  [6]  1111 00kk kkkk k111  (UNUSED)
#define OP_BRID         0xF407 //  [6]  1111 01kk kkkk k111  (UNUSED)
/* Bit and Bit-Test Instructions */
#define OP_SBI          0x9A00 //  [8]  1001 1010 AAAA Abbb
#define OP_CBI          0x9800 //  [8]  1001 1000 AAAA Abbb
#define OP_LSL          0x0C00 //  [6]  0000 11dd dddd dddd  (UNUSED)
#define OP_LSR          0x9406 //  [7]  1001 010d dddd 0110
#define OP_ROL          0x1C00 //  [6]  0001 11dd dddd dddd  (UNUSED)
#define OP_ROR          0x9407 //  [7]  1001 010d dddd 0111
#define OP_ASR          0x9405 //  [7]  1001 010d dddd 0101
#define OP_SWAP         0x9402 //  [7]  1001 010d dddd 0010
#define OP_BSET         0x9408 //  [9]  1001 0100 0sss 1000
#define OP_BCLR         0x9488 //  [9]  1001 0100 1sss 1000
#define OP_BST          0xFA00 //  [7]  1111 101d dddd 0bbb
#define OP_BLD          0xF800 //  [7]  1111 100d dddd 0bbb
#define OP_SEC          0x9408 // [16]  1001 0100 0000 1000  (UNUSED)
#define OP_CLC          0x9488 // [16]  1001 0100 1000 1000  (UNUSED)
#define OP_SEN          0x9428 // [16]  1001 0100 0010 1000  (UNUSED)
#define OP_CLN          0x94A8 // [16]  1001 0100 1010 1000  (UNUSED)
#define OP_SEZ          0x9418 // [16]  1001 0100 0001 1000  (UNUSED)
#define OP_CLZ          0x9498 // [16]  1001 0100 1001 1000  (UNUSED)
#define OP_SEI          0x9478 // [16]  1001 0100 0111 1000  (UNUSED)
#define OP_CLI          0x94F8 // [16]  1001 0100 1111 1000  (UNUSED)
#define OP_SES          0x9448 // [16]  1001 0100 0100 1000  (UNUSED)
#define OP_CLS          0x94C8 // [16]  1001 0100 1100 1000  (UNUSED)
#define OP_SEV          0x9468 // [16]  1001 0100 0110 1000  (UNUSED)
#define OP_CLV          0x94B8 // [16]  1001 0100 1011 1000  (UNUSED)
#define OP_SET          0x9438 // [16]  1001 0100 0011 1000  (UNUSED)
#define OP_CLT          0x94E8 // [16]  1001 0100 1110 1000  (UNUSED)
#define OP_SEH          0x9458 // [16]  1001 0100 0101 1000  (UNUSED)
#define OP_CLH          0x94D8 // [16]  1001 0100 1101 1000  (UNUSED)
/* Data Transfer Instructions */
#define OP_MOV          0x2C00 //  [6]  0010 11rd dddd rrrr
#define OP_MOVW         0x0100 //  [8]  0000 0001 dddd rrrr
#define OP_LDI          0xE000 //  [4]  1110 KKKK dddd KKKK
#define OP_LD_X         0x900C //  [7]  1001 000d dddd 1100
#define OP_LD_X_POSTINC 0x900D //  [7]  1001 000d dddd 1101
#define OP_LD_X_PREDEC  0x900E //  [7]  1001 000d dddd 1110
#define OP_LD_Y         0x8008 //  [7]  1000 000d dddd 1000
#define OP_LD_Y_POSTINC 0x9009 //  [7]  1001 000d dddd 1001
#define OP_LD_Y_PREDEC  0x900A //  [7]  1001 000d dddd 1010
#define OP_LD_Z         0x8000 //  [7]  1000 000d dddd 0000
#define OP_LD_Z_POSTINC 0x9001 //  [7]  1001 000d dddd 0001
#define OP_LD_Z_PREDEC  0x9002 //  [7]  1001 000d dddd 0010
#define OP_LDD_Y        0x8008 //  [*]  10q0 qq0d dddd 1qqq
#define OP_LDD_Z        0x8000 //  [*]  10q0 qq0d dddd 0qqq
#define OP_LDS          0x9000 //  [7]  1001 000d dddd 0000  kkkk kkkk kkkk kkkk
#define OP_ST_X         0x920C //  [7]  1001 001r rrrr 1100
#define OP_ST_X_POSTINC 0x920D //  [7]  1001 001r rrrr 1101
#define OP_ST_X_PREDEC  0x920E //  [7]  1001 001r rrrr 1110
#define OP_ST_Y         0x8208 //  [7]  1000 001r rrrr 1000
#define OP_ST_Y_POSTINC 0x9209 //  [7]  1001 001r rrrr 1001
#define OP_ST_Y_PREDEC  0x920A //  [7]  1001 001r rrrr 1010
#define OP_ST_Z         0x8200 //  [7]  1000 001r rrrr 0000
#define OP_ST_Z_POSTINC 0x9201 //  [7]  1001 001r rrrr 0001
#define OP_ST_Z_PREDEC  0x9202 //  [7]  1001 001r rrrr 0010
#define OP_STD_Y        0x8208 //  [*]  10q0 qq1r rrrr 1qqq
#define OP_STD_Z        0x8200 //  [*]  10q0 qq1r rrrr 0qqq
#define OP_STS          0x9200 //  [7]  1001 001d dddd 0000  kkkk kkkk kkkk kkkk
#define OP_LPM_R0       0x95C8 // [16]  1001 0101 1100 1000
#define OP_LPM          0x9004 //  [7]  1001 0ddd dddd 0100
#define OP_LPM_POSTINC  0x9005 //  [7]  1001 0ddd dddd 0101
#define OP_SPM          0x95E8 // [16]  1001 0101 1110 1000
#define OP_IN           0xB000 //  [5]  1011 0AAd dddd AAAA
#define OP_OUT          0xB800 //  [5]  1011 1AAr rrrr AAAA
#define OP_PUSH         0x920F //  [7]  1001 001d dddd 1111
#define OP_POP          0x900F //  [7]  1001 000d dddd 1111
/* MCU Control Instructions */
#define OP_NOP          0x0000 // [16]  0000 0000 0000 0000
#define OP_SLEEP        0x9588 // [16]  1001 0101 1000 1000
#define OP_WDR          0x95A8 // [16]  1001 0101 1010 1000
#define OP_BREAK        0x9598 // [16]  1001 0101 1001 1000

/*******************************************************************************
 * Op Masks
 ******************************************************************************/
#define OP_MASK_4   0xF000 // 1111 .... .... ....
#define OP_MASK_5   0xF800 // 1111 1... .... ....
#define OP_MASK_6   0xFC00 // 1111 11.. .... ....
#define OP_MASK_7_1 0xFE08 // 1111 111. .... 1...
#define OP_MASK_7_3 0xFE0E // 1111 111. .... 111.
#define OP_MASK_7_4 0xFE0F // 1111 111. .... 1111
#define OP_MASK_8   0xFF00 // 1111 1111 .... ....
#define OP_MASK_8_4 0xFF0F // 1111 1111 .... 1111
#define OP_MASK_9_1 0xFF88 // 1111 1111 1... 1...
#define OP_MASK_9_4 0xFF8F // 1111 1111 1... 1111
#define OP_MASK_Q   0xD208 // 11.1 ..1. .... 1...

/*******************************************************************************
 * Op Utils
 ******************************************************************************/
#define IS_32BIT_OP(OP)                                                                                     \
    (((OP) & OP_MASK_7_3) == OP_JMP || ((OP) & OP_MASK_7_3) == OP_CALL || ((OP) & OP_MASK_7_4) == OP_STS || \
     ((OP) & OP_MASK_7_4) == OP_LDS)

#define IS_IO_SPACE(ADDR) ((ADDR) >= 0x20 && (ADDR) < 0x60);

#define MSK(OP, MSK)     ((OP) & (MSK))           /* mask */
#define MSH(OP, MSK, SH) (((OP) & (MSK)) >> (SH)) /* mask with shift */

/*******************************************************************************
 * IO Register Offsets
 ******************************************************************************/
#define REG_PINB   0x23
#define REG_DDRB   0x24
#define REG_PORTB  0x25
#define REG_PINC   0x26
#define REG_DDRC   0x27
#define REG_PORTC  0x28
#define REG_PIND   0x29
#define REG_DDRD   0x2A
#define REG_PORTD  0x2B
// RESERVED 0x2C - 0x34
#define REG_TIFR0  0x35
#define REG_TIFR1  0x36
#define REG_TIFR2  0x37
// RESERVED 0x38 - 0x3A
#define REG_PCIFR  0x3B
#define REG_EIFR   0x3C
#define REG_EIMSK  0x3D
#define REG_GPIOR0 0x3E
#define REG_EECR   0x3F
#define REG_EEDR   0x40
#define REG_EEARL  0x41
#define REG_EEARH  0x42
#define REG_GTCCR  0x43
#define REG_TCCR0A 0x44
#define REG_TCCR0B 0x45
#define REG_TCNT0  0x46
#define REG_OCR0A  0x47
#define REG_OCR0B  0x48
// RESERVED 0x49
#define REG_GPIOR1 0x4A
#define REG_GPIOR2 0x4B
#define REG_SPCR   0x4C
#define REG_SPSR   0x4D
#define REG_SPDR   0x4E
// RESERVED 0x4F
#define REG_ACSR   0x50
// RESERVED 0x51 - 0x52
#define REG_SMCR   0x53
#define REG_MCUSR  0x54
#define REG_MCUCR  0x55
// RESERVED 0x56
#define REG_SPMCSR 0x57
// RESERVED 0x58 - 0x5C
#define REG_SPL    0x5D
#define REG_SPH    0x5E
#define REG_SREG   0x5F
#define REG_WDTCSR 0x60
#define REG_CLKPR  0x61
// RESERVED 0x62 - 0x63
#define REG_PRR    0x64
// RESERVED 0x65
#define REG_OSCCAL 0x66
// RESERVED 0x67
#define REG_PCICR  0x68
#define REG_EICRA  0x69
// RESERVED 0x6A
#define REG_PCMSK0 0x6B
#define REG_PCMSK1 0x6C
#define REG_PCMSK2 0x6D
#define REG_TIMSK0 0x6E
#define REG_TIMSK1 0x6F
#define REG_TIMSK2 0x70
// RESERVED 0x71 - 0x77
#define REG_ADCL   0x78
#define REG_ADCH   0x79
#define REG_ADCSRA 0x7A
#define REG_ADCSRB 0x7B
#define REG_ADMUX  0x7C
// RESERVED 0x7D
#define REG_DIDR0  0x7E
#define REG_DIDR1  0x7F
#define REG_TCCR1A 0x80
#define REG_TCCR1B 0x81
#define REG_TCCR1C 0x82
// RESERVED 0x83
#define REG_TCNT1L 0x84
#define REG_TCNT1H 0x85
#define REG_ICR1L  0x86
#define REG_ICR1H  0x87
#define REG_OCR1AL 0x88
#define REG_OCR1AH 0x89
#define REG_OCR1BL 0x8A
#define REG_OCR1BH 0x8B
// RESERVED 0x8C - 0xAF
#define REG_TCCR2A 0xB0
#define REG_TCCR2B 0xB1
#define REG_TCNT2  0xB2
#define REG_OCR2A  0xB3
#define REG_OCR2B  0xB4
// RESERVED 0xB5
#define REG_ASSR   0xB6
// RESERVED 0xB7
#define REG_TWBR   0xB8
#define REG_TWSR   0xB9
#define REG_TWAR   0xBA
#define REG_TWDR   0xBB
#define REG_TWCR   0xBC
#define REG_TWAMR  0xBD
// RESERVED 0xBE - 0xBF
#define REG_UCSR0A 0xC0
#define REG_UCSR0B 0xC1
#define REG_UCSR0C 0xC2
// RESERVED 0xC3
#define REG_UBRR0L 0xC4
#define REG_UBRR0H 0xC5
#define REG_UDR0   0xC6
// RESERVED 0xC7 - 0xFF

// limited set of useful bits
#define BIT_TOV0  0
#define BIT_OCF0A 1
#define BIT_OCF0B 2
#define BIT_TOV1  0
#define BIT_ICF1  5
#define BIT_OCF1A 1
#define BIT_OCF1B 2
#define BIT_TOV2  0
#define BIT_OCF2A 1
#define BIT_OCF2B 2
#define BIT_PCIF0 0
#define BIT_PCIF1 1
#define BIT_PCIF2 2
#define BIT_INTF0 0
#define BIT_INTF1 1
#define BIT_INT0  0
#define BIT_INT1  1
#define BIT_EERE  0
#define BIT_EEPE  1
#define BIT_EEMPE 2
#define BIT_EERIE 3
#define BIT_EEPM0 4
#define BIT_EEPM1 5

/*******************************************************************************
 * Sleep Modes
 *
 * Bit 0 : SE (sleep enable)
 * Bit 1 : SM0
 * Bit 2 : SM1
 * Bit 3 : SM2
 ******************************************************************************/
#define SLEEP_IDLE             0x01 // 0001
#define SLEEP_ADC_NR           0x03 // 0011
#define SLEEP_POWER_DOWN       0x05 // 0101
#define SLEEP_POWER_SAVE       0x07 // 0111
// RESERVED 0x09 - 0x0A
#define SLEEP_STANDBY          0x0C // 1101  (UNUSED)
#define SLEEP_EXTERNAL_STANDBY 0x0F // 1111  (UNUSED)

/*******************************************************************************
 * Interrupt Vectors
 ******************************************************************************/
#define IV_RESET        0x0000 // External pin, power-on reset, brown-out reset and watchdog system reset
#define IV_INT0         0x0002 // External interrupt request 0
#define IV_INT1         0x0004 // External interrupt request 1
#define IV_PCINT0       0x0006 // Pin change interrupt request 0
#define IV_PCINT1       0x0008 // Pin change interrupt request 1
#define IV_PCINT2       0x000A // Pin change interrupt request 2
#define IV_WDT          0x000C // Watchdog time-out interrupt
#define IV_TIMER2_COMPA 0x000E // Timer/Counter2 compare match A
#define IV_TIMER2_COMPB 0x0010 // Timer/Counter2 compare match B
#define IV_TIMER2_OVF   0x0012 // Timer/Counter2 overflow
#define IV_TIMER1_CAPT  0x0014 // Timer/Counter1 capture event
#define IV_TIMER1_COMPA 0x0016 // Timer/Counter1 compare match A
#define IV_TIMER1_COMPB 0x0018 // Timer/Counter1 compare match B
#define IV_TIMER1_OVF   0x001A // Timer/Counter1 overflow
#define IV_TIMER0_COMPA 0x001C // Timer/Counter0 compare match A
#define IV_TIMER0_COMPB 0x001E // Timer/Counter0 compare match B
#define IV_TIMER0_OVF   0x0020 // Timer/Counter0 overflow
#define IV_SPI_STC      0x0022 // SPI serial transfer complete
#define IV_USART_RX     0x0024 // USART Rx complete
#define IV_USART_UDRE   0x0026 // USART, data register empty
#define IV_USART_TX     0x0028 // USART, Tx complete
#define IV_ADC          0x002A // ADC conversion complete
#define IV_EE_READY     0x002C // EEPROM ready
#define IV_ANALONG_COMP 0x002E // Analog comparator
#define IV_TWI          0x0030 // 2-wire serial interface
#define IV_SPM_READY    0x0032 // Store program memory ready

#endif // _AVR__AVR_DEFS_H_
