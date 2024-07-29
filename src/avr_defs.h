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

#pragma once

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
 * If an opcode has a trailing '_' it cannot be used by itself and a bitmask
 * must be or'ed with it e.g. OP_LD_ needs a OP_LD_* mask.
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
#define OP_ADD    0x0C00 //  [6]  0000 11rd dddd rrrr
#define OP_ADC    0x1C00 //  [6]  0001 11rd dddd rrrr
#define OP_ADIW   0x9600 //  [8]  1001 0110 KKdd KKKK
#define OP_SUB    0x1800 //  [6]  0001 10rd dddd rrrr
#define OP_SUBI   0x5000 //  [4]  0101 KKKK dddd KKKK
#define OP_SBC    0x0800 //  [6]  0000 10rd dddd rrrr
#define OP_SBCI   0x4000 //  [4]  0100 KKKK dddd KKKK
#define OP_SBIW   0x9700 //  [6]  1001 0111 KKdd KKKK
#define OP_AND    0x2000 //  [6]  0010 00rd dddd rrrr
#define OP_ANDI   0x7000 //  [4]  0111 KKKK dddd KKKK
#define OP_OR     0x2800 //  [6]  0010 10rd dddd rrrr
#define OP_ORI    0x6000 //  [4]  0110 KKKK dddd KKKK
#define OP_EOR    0x2400 //  [6]  0010 01rd dddd rrrr
#define OP_COM    0x9400 //  [9]  1001 010d dddd 0000
#define OP_NEG    0x9401 //  [7]  1001 010d dddd 0001
#define OP_SBR    0x6000 //  [4]  0110 KKKK dddd KKKK  (UNUSED)
#define OP_CBR    0x7000 //  [4]  0111 KKKK dddd KKKK  (UNUSED)
#define OP_INC    0x9403 //  [7]  1001 010d dddd 0011
#define OP_DEC    0x940A //  [7]  1001 010d dddd 1010
#define OP_TST    0x2000 //  [6]  0010 00dd dddd dddd  (UNUSED)
#define OP_CLR    0x2400 //  [6]  0010 01dd dddd dddd  (UNUSED)
#define OP_SER    0xEF0F //  [8]  1110 1111 dddd 1111
#define OP_MUL    0x9C00 //  [6]  1001 11rd dddd rrrr
#define OP_MULS   0x0200 //  [8]  0000 0010 dddd rrrr
#define OP_MULSU  0x0300 //  [9]  0000 0011 0ddd 0rrr
#define OP_FMUL   0x0308 //  [9]  0000 0011 0ddd 1rrr
#define OP_FMULS  0x0380 //  [9]  0000 0011 1ddd 0rrr
#define OP_FMULSU 0x0388 //  [9]  0000 0011 1ddd 1rrr
/* Branch Instructions */
#define OP_RJMP   0x____
#define OP_IJMP   0x9409 // [16]  1001 0100 0000 1001
#define OP_JMP    0x940C //  [7]  1001 010k kkkk 110k  kkkk kkkk kkkk kkkk
#define OP_RCALL  0x____
#define OP_ICALL  0x9509 // [16]  1001 0101 0000 1001
#define OP_CALL   0x940E //  [7]  1001 010k kkkk 111k  kkkk kkkk kkkk kkkk
#define OP_RET    0x____
#define OP_RETI   0x____
#define OP_CPSE   0x1000 //  [6]  0001 00rd dddd rrrr
#define OP_CP     0x1400 //  [6]  0001 01rd dddd rrrr
#define OP_CPC    0x0400 //  [6]  0000 01rd dddd rrrr
#define OP_CPI    0x3000 //  [4]  0011 KKKK dddd KKKK
#define OP_SBRC   0x____
#define OP_SBRS   0x____
#define OP_SBIC   0x____
#define OP_SBIS   0x____
#define OP_BRBS   0xF000 //  [6]  1111 00kk kkkk ksss
#define OP_BRBC   0xF400 //  [6]  1111 01kk kkkk ksss
#define OP_BREQ   0xF001 //  [6]  1111 00kk kkkk k001  (UNUSED)
#define OP_BRNE   0xF401 //  [6]  1111 01kk kkkk k001  (UNUSED)
#define OP_BRCS   0xF000 //  [6]  1111 00kk kkkk k000  (UNUSED)
#define OP_BRCC   0xF400 //  [6]  1111 01kk kkkk k000  (UNUSED)
#define OP_BRSH   0xF400 //  [6]  1111 01kk kkkk k000  (UNUSED)
#define OP_BRLO   0xF000 //  [6]  1111 00kk kkkk k000  (UNUSED)
#define OP_BRMI   0xF002 //  [6]  1111 00kk kkkk k010  (UNUSED)
#define OP_BRPL   0xF402 //  [6]  1111 01kk kkkk k010  (UNUSED)
#define OP_BRGE   0xF404 //  [6]  1111 01kk kkkk k100  (UNUSED)
#define OP_BRLT   0xF004 //  [6]  1111 00kk kkkk k100  (UNUSED)
#define OP_BRHS   0xF005 //  [6]  1111 00kk kkkk k101  (UNUSED)
#define OP_BRHC   0xF405 //  [6]  1111 01kk kkkk k101  (UNUSED)
#define OP_BRTS   0xF006 //  [6]  1111 00kk kkkk k110  (UNUSED)
#define OP_BRTC   0xF406 //  [6]  1111 01kk kkkk k110  (UNUSED)
#define OP_BRVS   0xF003 //  [6]  1111 00kk kkkk k011  (UNUSED)
#define OP_BRVC   0xF403 //  [6]  1111 01kk kkkk k011  (UNUSED)
#define OP_BRIE   0xF007 //  [6]  1111 00kk kkkk k111  (UNUSED)
#define OP_BRID   0xF407 //  [6]  1111 01kk kkkk k111  (UNUSED)
/* Bit and Bit-Test Instructions */
#define OP_SBI    0x____
#define OP_CBI    0x9800 //  [8]  1001 1000 AAAA Abbb
#define OP_LSL    0x____
#define OP_LSR    0x____
#define OP_ROL    0x____
#define OP_ROS    0x____
#define OP_ASR    0x9405 //  [7]  1001 010d dddd 0101
#define OP_SWAP   0x____
#define OP_BSET   0x9408 //  [9]  1001 0100 0sss 1000
#define OP_BCLR   0x9488 //  [9]  1001 0100 1sss 1000
#define OP_BST    0xFA00 //  [7]  1111 101d dddd 0bbb
#define OP_BLD    0xF800 //  [7]  1111 100d dddd 0bbb
#define OP_SEC    0x____
#define OP_CLC    0x9488 // [16]  1001 0100 1000 1000  (UNUSED)
#define OP_SEN    0x____
#define OP_CLN    0x94A8 // [16]  1001 0100 1010 1000  (UNUSED)
#define OP_SEZ    0x____
#define OP_CLZ    0x9498 // [16]  1001 0100 1001 1000  (UNUSED)
#define OP_SEI    0x____
#define OP_CLI    0x94F8 // [16]  1001 0100 1111 1000  (UNUSED)
#define OP_SES    0x____
#define OP_CLS    0x94C8 // [16]  1001 0100 1100 1000  (UNUSED)
#define OP_SEV    0x____
#define OP_CLV    0x94B8 // [16]  1001 0100 1011 1000  (UNUSED)
#define OP_SET    0x____
#define OP_CLT    0x94E8 // [16]  1001 0100 1110 1000  (UNUSED)
#define OP_SEH    0x____
#define OP_CLH    0x94D8 // [16]  1001 0100 1101 1000  (UNUSED)
/* Data Transfer Instructions */
#define OP_MOV    0x____
#define OP_MOVW   0x____
#define OP_LDI    0xE000 //  [4]  1110 KKKK dddd KKKK
#define OP_LD_    0x8000 //  [7]  1000 000d dddd 0000
#define OP_LDD    0x____
#define OP_LDS    0x9000 //  [7]  1001 000d dddd 0000  kkkk kkkk kkkk kkkk
#define OP_ST     0x____
#define OP_STD    0x____
#define OP_STS    -1
#define OP_LPM_   0x9000 //  [*]  1001 0000 0000 0000 // TODO FIXME
#define OP_SPM    0x____
#define OP_IN     0xB000 //  [5]  1011 0AAd dddd AAAA
#define OP_OUT    0x____
#define OP_PUSH   0x____
#define OP_POP    0x____
/* MCU Control Instructions */
#define OP_NOP    0x____
#define OP_SLEEP  0x____
#define OP_WDR    0x____
#define OP_BREAK  0x9598 // [16]  1001 0101 1001 1000

/* Extensions (not used by ATmega328P) */
#define OP_LAC 0x9206 //  [7]  1001 001r rrrr 0110
#define OP_LAS 0x9205 //  [7]  1001 001r rrrr 0101
#define OP_LAT 0x9207 //  [7]  1001 001r rrrr 0111

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

/*******************************************************************************
 * Op Utils
 ******************************************************************************/
#define IS_32BIT_OP(OP)                                                                                     \
    (((OP) & OP_MASK_7_3) == OP_JMP || ((OP) & OP_MASK_7_3) == OP_CALL || ((OP) & OP_MASK_7_4) == OP_STS || \
     ((OP) & OP_MASK_7_4) == OP_LDS)

#define OP_LD_X         0x100C // ...1 .... .... 11..
#define OP_LD_X_POSTINC 0x100D // ...1 .... .... 11.1
#define OP_LD_X_PREDEC  0x100E // ...1 .... .... 111.
#define OP_LD_Y         0x0008 // .... .... .... 1...
#define OP_LD_Y_POSTINC 0x1009 // ...1 .... .... 1..1
#define OP_LD_Y_PREDEC  0x100A // ...1 .... .... 1.1.
#define OP_LD_Z         0x0000 // .... .... .... ....
#define OP_LD_Z_POSTINC 0x1001 // ...1 .... .... ...1
#define OP_LD_Z_PREDEC  0x1002 // ...1 .... .... .11.

#define OP_LPM_Z_R0
#define OP_LPM_Z
#define OP_LPM_Z_POSTINC

/*******************************************************************************
 * Addressing
 ******************************************************************************/
#define GET_REG_DIRECT_DST(OP) (((OP) & 0x01F0) >> 4)
#define GET_REG_DIRECT_SRC(OP) (((OP) & 0x0200) >> 5 | ((OP) & 0x000F))

#define GET_REG_IMMEDIATE_DST(OP)   (((OP) & 0x00F0) >> 4)
#define GET_REG_IMMEDIATE_CONST(OP) ((((OP) & 0x0F00) >> 4) | ((OP) & 0x000F))
