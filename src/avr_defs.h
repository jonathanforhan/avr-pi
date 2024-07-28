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

#include "defs.h"

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
#define OP_ADC   0b0001110000000000 //  [6]  0001 11rd dddd rrrr
#define OP_ADD   0b0000110000000000 //  [6]  0000 11rd dddd rrrr
#define OP_ADIW  0b1001011000000000 //  [8]  1001 0110 KKdd KKKK
#define OP_AND   0b0010000000000000 //  [6]  0010 00rd dddd rrrr
#define OP_ANDI  0b0111000000000000 //  [4]  0111 KKKK dddd KKKK
#define OP_ASR   0b1001010000000101 //  [7]  1001 010d dddd 0101
#define OP_BCLR  0b1001010000000101 //  [9]  1001 010d dddd 0101
#define OP_BLD   0b1111100000000000 //  [7]  1111 100d dddd 0bbb
#define OP_BRBC  0b1111010000000000 //  [6]  1111 01kk kkkk ksss
#define OP_BRBS  0b1111000000000000 //  [6]  1111 00kk kkkk ksss
#define OP_BRCC  0b1111010000000000 //  [6]  1111 01kk kkkk k000  (UNUSED)
#define OP_BRCS  0b1111000000000000 //  [6]  1111 00kk kkkk k000  (UNUSED)
#define OP_BREAK 0b1001010110011000 // [16]  1001 0101 1001 1000  (UNUSED)
#define OP_BREQ  0b1111000000000001 //  [6]  1111 00kk kkkk k001  (UNUSED)
#define OP_BRGE  0b1111010000000100 //  [6]  1111 01kk kkkk k100  (UNUSED)
#define OP_BRHC  0b1111010000000101 //  [6]  1111 01kk kkkk k101  (UNUSED)
#define OP_BRHS  0b1111000000000101 //  [6]  1111 00kk kkkk k101  (UNUSED)
#define OP_BRID  0b1111010000000111 //  [6]  1111 01kk kkkk k111  (UNUSED)
#define OP_BRIE  0b1111000000000111 //  [6]  1111 00kk kkkk k111  (UNUSED)
#define OP_BRLO  0b1111000000000000 //  [6]  1111 00kk kkkk k000  (UNUSED)
#define OP_BRLT  0b1111000000000100 //  [6]  1111 00kk kkkk k100  (UNUSED)
#define OP_BRMI  0b1111000000000010 //  [6]  1111 00kk kkkk k010  (UNUSED)
#define OP_BRNE  0b1111010000000001 //  [6]  1111 01kk kkkk k001  (UNUSED)
#define OP_BRPL  0b1111010000000010 //  [6]  1111 01kk kkkk k010  (UNUSED)
#define OP_BRSH  0b1111010000000000 //  [6]  1111 01kk kkkk k000  (UNUSED)
#define OP_BRTC  0b1111010000000110 //  [6]  1111 01kk kkkk k110  (UNUSED)
#define OP_BRTS  0b1111000000000110 //  [6]  1111 00kk kkkk k110  (UNUSED)
#define OP_BRVC  0b1111010000000011 //  [6]  1111 01kk kkkk k011  (UNUSED)
#define OP_BRVS  0b1111000000000011 //  [6]  1111 00kk kkkk k011  (UNUSED)
#define OP_BSET  0b1001010000001000 //  [9]  1001 0100 0sss 1000
#define OP_BST   0b1111101000000000 //  [7]  1111 101d dddd 0bbb
#define OP_CALL  0b1001010000001110 //  [7]  1001 010k kkkk 111k  kkkk kkkk kkkk kkkk

#define OP_JMP 0b1001010000001100 // [7]  1001 010k kkkk 110k  kkkk kkkk kkkk kkkk

/*******************************************************************************
 * Op Masks
 ******************************************************************************/
#define OP_MASK_4   0b1111000000000000 // 0b1111............
#define OP_MASK_5   0b1111100000000000 // 0b11111...........
#define OP_MASK_6   0b1111110000000000 // 0b111111..........
#define OP_MASK_7_1 0b1111111000001000 // 0b1111111.....1...
#define OP_MASK_7_3 0b1111111000001110 // 0b1111111.....111.
#define OP_MASK_7_4 0b1111111000001111 // 0b1111111.....1111
#define OP_MASK_8   0b1111111100000000 // 0b11111111........
#define OP_MASK_9_4 0b1111111110001111 // 0b111111111...1111

/*******************************************************************************
 * Addressing
 ******************************************************************************/
#define GET_REG_DIRECT_DST(OP) ((OP & 0b0000000111110000) >> 4)
#define GET_REG_DIRECT_SRC(OP) ((OP & 0b0000000000001111) | (GET_BIT(OP, 9) << 4))

#define GET_IO_DIRECT_REG(OP)  // TODO
#define GET_IO_DIRECT_ADDR(OP) // TODO

#define GET_DATA_DIRECT(OP)             // TODO
#define GET_DATA_INDIRECT(OP)           // TODO
#define GET_DATA_INDIRECT_DISPLACED(OP) // TODO
#define GET_DATA_INDIRECT_PREDEC(OP)    // TODO
#define GET_DATA_INDIRECT_POSTDEC(OP)   // TODO

#define GET_PROG_DIRECT_ADDR(HI, LO) TO_U32((((HI & (~OP_MASK_7_3)) >> 3) | (HI & 1)), LO)
#define GET_PROG_INDIRECT_ADDR()     // TODO
#define GET_PROG_RELATIVE_ADDR()     // TODO
