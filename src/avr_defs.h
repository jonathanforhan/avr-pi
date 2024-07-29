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
 * NOT IMPLEMENTED: Used for AVR op extensions that are not implemented
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

#define OP_ADC    0x1C00 //  [6]  0001 11rd dddd rrrr
#define OP_ADD    0x0C00 //  [6]  0000 11rd dddd rrrr
#define OP_ADIW   0x9600 //  [8]  1001 0110 KKdd KKKK
#define OP_AND    0x2000 //  [6]  0010 00rd dddd rrrr
#define OP_ANDI   0x7000 //  [4]  0111 KKKK dddd KKKK
#define OP_ASR    0x9405 //  [7]  1001 010d dddd 0101
#define OP_BCLR   0x9488 //  [9]  1001 0100 1sss 1000
#define OP_BLD    0xF800 //  [7]  1111 100d dddd 0bbb
#define OP_BRBC   0xF400 //  [6]  1111 01kk kkkk ksss
#define OP_BRBS   0xF000 //  [6]  1111 00kk kkkk ksss
#define OP_BRCC   0xF400 //  [6]  1111 01kk kkkk k000  (UNUSED)
#define OP_BRCS   0xF000 //  [6]  1111 00kk kkkk k000  (UNUSED)
#define OP_BREAK  0x9598 // [16]  1001 0101 1001 1000
#define OP_BREQ   0xF001 //  [6]  1111 00kk kkkk k001  (UNUSED)
#define OP_BRGE   0xF404 //  [6]  1111 01kk kkkk k100  (UNUSED)
#define OP_BRHC   0xF405 //  [6]  1111 01kk kkkk k101  (UNUSED)
#define OP_BRHS   0xF005 //  [6]  1111 00kk kkkk k101  (UNUSED)
#define OP_BRID   0xF407 //  [6]  1111 01kk kkkk k111  (UNUSED)
#define OP_BRIE   0xF007 //  [6]  1111 00kk kkkk k111  (UNUSED)
#define OP_BRLO   0xF000 //  [6]  1111 00kk kkkk k000  (UNUSED)
#define OP_BRLT   0xF004 //  [6]  1111 00kk kkkk k100  (UNUSED)
#define OP_BRMI   0xF002 //  [6]  1111 00kk kkkk k010  (UNUSED)
#define OP_BRNE   0xF401 //  [6]  1111 01kk kkkk k001  (UNUSED)
#define OP_BRPL   0xF402 //  [6]  1111 01kk kkkk k010  (UNUSED)
#define OP_BRSH   0xF400 //  [6]  1111 01kk kkkk k000  (UNUSED)
#define OP_BRTC   0xF406 //  [6]  1111 01kk kkkk k110  (UNUSED)
#define OP_BRTS   0xF006 //  [6]  1111 00kk kkkk k110  (UNUSED)
#define OP_BRVC   0xF403 //  [6]  1111 01kk kkkk k011  (UNUSED)
#define OP_BRVS   0xF003 //  [6]  1111 00kk kkkk k011  (UNUSED)
#define OP_BSET   0x9408 //  [9]  1001 0100 0sss 1000
#define OP_BST    0xFA00 //  [7]  1111 101d dddd 0bbb
#define OP_CALL   0x940E //  [7]  1001 010k kkkk 111k  kkkk kkkk kkkk kkkk
#define OP_CBI    0x9800 //  [8]  1001 1000 AAAA Abbb
#define OP_CBR    0x7000 //  [4]  0111 KKKK dddd KKKK  (UNUSED)
#define OP_CLC    0x9488 // [16]  1001 0100 1000 1000  (UNUSED)
#define OP_CLH    0x94D8 // [16]  1001 0100 1101 1000  (UNUSED)
#define OP_CLI    0x94F8 // [16]  1001 0100 1111 1000  (UNUSED)
#define OP_CLN    0x94A8 // [16]  1001 0100 1010 1000  (UNUSED)
#define OP_CLR    0x2400 //  [6]  0010 01dd dddd dddd  (UNUSED)
#define OP_CLS    0x94C8 // [16]  1001 0100 1100 1000  (UNUSED)
#define OP_CLT    0x94E8 // [16]  1001 0100 1110 1000  (UNUSED)
#define OP_CLV    0x94B8 // [16]  1001 0100 1011 1000  (UNUSED)
#define OP_CLZ    0x9498 // [16]  1001 0100 1001 1000  (UNUSED)
#define OP_COM    0x9400 //  [9]  1001 010d dddd 0000
#define OP_CP     0x1400 //  [6]  0001 01rd dddd rrrr
#define OP_CPC    0x0400 //  [6]  0000 01rd dddd rrrr
#define OP_CPI    0x3000 //  [4]  0011 KKKK dddd KKKK
#define OP_CPSE   0x1000 //  [6]  0001 00rd dddd rrrr
#define OP_DEC    0x940A //  [7]  1001 010d dddd 1010
#define OP_DES    0x940B //  [8]  1001 0100 KKKK 1011  (NOT IMPLEMENTED)
#define OP_EICALL 0x9519 // [16]  1001 0101 0001 1001  (NOT IMPLEMENTED)
#define OP_EIJMP  0x9419 // [16]  1001 0100 0001 1001  (NOT IMPLEMENTED)
#define OP_ELPM   0x95D8 // [16]  1001 0101 1101 1000  (NOT IMPLEMENTED)
#define OP_EOR    0x2400 //  [6]  0010 01rd dddd rrrr
#define OP_FMUL   0x0308 //  [9]  0000 0011 0ddd 1rrr  (NOT IMPLEMENTED)
#define OP_FMULS  0x0380 //  [9]  0000 0011 1ddd 0rrr  (NOT IMPLEMENTED)
#define OP_FMULSU 0x0388 //  [9]  0000 0011 1ddd 1rrr  (NOT IMPLEMENTED)
#define OP_ICALL  0x9509 // [16]  1001 0101 0000 1001
#define OP_IJMP   0x9409 // [16]  1001 0100 0000 1001
#define OP_IN     0xB000 //  [5]  1011 0AAd dddd AAAA
#define OP_INC    0x9403 //  [7]  1001 010d dddd 0011
#define OP_JMP    0x940C //  [7]  1001 010k kkkk 110k  kkkk kkkk kkkk kkkk
#define OP_LAC    0x9206 //  [7]  1001 001r rrrr 0110
#define OP_LAS    0x9205 //  [7]  1001 001r rrrr 0101
#define OP_LAT    0x9207 //  [7]  1001 001r rrrr 0111

#define OP_LDS -1 // TODO
#define OP_STS -2 // TODO

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
#define OP_MASK_9_4 0xFF8F // 1111 1111 1... 1111

/*******************************************************************************
 * Op Utils
 ******************************************************************************/
#define IS_32BIT_OP(OP)                                                                                     \
    (((OP) & OP_MASK_7_3) == OP_JMP || ((OP) & OP_MASK_7_3) == OP_CALL || ((OP) & OP_MASK_7_4) == OP_STS || \
     ((OP) & OP_MASK_7_4) == OP_LDS)

/*******************************************************************************
 * Addressing
 ******************************************************************************/
#define GET_REG_DIRECT_DST(OP) ((OP & 0x01F0) >> 4)
#define GET_REG_DIRECT_SRC(OP) ((OP & 0x0200) >> 5 | (OP & 0x000F))
