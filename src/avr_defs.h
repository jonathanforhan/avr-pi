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
 * hex record type
 ******************************************************************************/
#define DATA_RECORD                  0
#define EOF_RECORD                   1
#define EXTENDED_SEGMENT_ADDR_RECORD 2
#define START_SEGMENT_ADDR_RECORD    3
#define EXTENDED_LINEAR_ADDR_RECORD  4
#define START_LINEAR_ADDR_RECORD     5

/*******************************************************************************
 * status reg bits
 ******************************************************************************/
#define SREG_I 7
#define SREG_T 6
#define SREG_H 5
#define SREG_S 4
#define SREG_V 3
#define SREG_N 2
#define SREG_Z 1
#define SREG_C 0

/*******************************************************************************
 * opcodes
 ******************************************************************************/
#define OP_ADC 0b0001110000000000 // [6]  0001 11rd dddd rrrr
#define OP_JMP 0b1001010000001100 // [7]  1001 010k kkkk 110k  kkkk kkkk kkkk kkkk

/*******************************************************************************
 * op support
 ******************************************************************************/
#define OP_TYPE_JUMP 0b1001010000000000 // 0b1001010.........

#define OP_MASK_6    0b1111110000000000 // 0b111111..........
#define OP_MASK_7    0b1111111000000000 // 0b1111111.........
#define OP_MASK_JUMP 0b1111111000001110 // 0b1111111.....111.

/*******************************************************************************
 * addressing
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

#define GET_PROG_DIRECT_ADDR(HI, LO) TO_U32((((HI & (~OP_MASK_JUMP)) >> 3) | (HI & 1)), LO)
#define GET_PROG_INDIRECT_ADDR()     // TODO
#define GET_PROG_RELATIVE_ADDR()     // TODO
