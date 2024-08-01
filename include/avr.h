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

/**
 * @file avr.h
 * @brief Emulates ATmega328P.
 */

#ifndef _AVR__AVR_H_
#define _AVR__AVR_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def AVR_MCU_SREG_OFFSET
 * @brief Data offset used by status register.
 */
#define AVR_MCU_SREG_OFFSET 0x005F

/**
 * @def AVR_MCU_SP_OFFSET
 * @brief Data offset used by stack pointer.
 */
#define AVR_MCU_SP_OFFSET 0x005D

/**
 * @def AVR_MCU_REG_OFFSET
 * @brief Data offset used by registers.
 */
#define AVR_MCU_REG_OFFSET 0x0000

/**
 * @def AVR_MCU_IO_REG_OFFSET
 * @brief Data offset used by io registers.
 */
#define AVR_MCU_IO_REG_OFFSET 0x0020

/**
 * @def AVR_MCU_EXT_IO_REG_OFFSET
 * @brief Data offset used by extended io registers.
 */
#define AVR_MCU_EXT_IO_REG_OFFSET 0x0060

/**
 * @def AVR_MCU_SRAM_OFFSET
 * @brief Data offset used by internal sram.
 */
#define AVR_MCU_SRAM_OFFSET 0x0100

/**
 * @def AVR_MCU_DATA_SIZE
 * @brief Size of data section in MCU, 2304 bytes.
 */
#define AVR_MCU_DATA_SIZE 0x08FF

/**
 * @def AVR_MCU_FLASH_SIZE
 * @brief Size of MCU flash section in bytes, 32KB.
 */
#define AVR_MCU_FLASH_SIZE 0x8000

/**
 * @def AVR_MCU_EEPROM_SIZE
 * @brief Size of eeprom section in MCU, 1KB.
 */
#define AVR_MCU_EEPROM_SIZE 0x08FF

/**
 * @def AVR_MCU_RAMEND
 * @brief End of SRAM.
 */
#define AVR_MCU_RAMEND 0x0800

/**
 * @brief Result type for avr_* functions.
 */
typedef enum AVR_Result {
    /** @brief Returned on success. */
    AVR_OK = 0,

    /** @brief Returned on error. */
    AVR_ERROR = 1,
} AVR_Result;

/**
 * @brief AVR Microcontroller.
 *
 * AVR 8-bit Status Register contain arithemetic state.
 *
 * SREG
 * +---+---+---+---+---+---+---+---+
 * | C | Z | N | V | S | H | T | I | flag
 * +---+---+---+---+---+---+---+---+
 *   0   1   2   3   4   5   6   7   bits
 *
 * C : Carry Flag
 * Z : Zero Flag
 * N : Negative Flag
 * V : Two's complement overflow indicator
 * S : N ^ V, for signed tests
 * H : Hald Carry Flag
 * T : Transfer bit used by BLD and BST instructions
 * I : Global Interrupt Enable/Disable Flag
 *
 * AVR CPU General Purpose Working Registers
 *
 * +-----+-----+-----+-----+-----+-----+-----+------+----+-----+-----+-----+-----+-----+-----+-----+
 * | R0  | R1  | R2  | R3  | R4  | R5  | R6  | R7  | R8  | R9  | R10 | R11 | R12 | R13 | R14 | R15 |
 * +-----+-----+-----+-----+-----+-----+-----+------+----+-----+-----+-----+-----+-----+-----+-----+
 * | R16 | R17 | R18 | R19 | R20 | R21 | R22 | R23 | R24 | R25 | R26 | R27 | R28 | R29 | R30 | R31 |
 * +-----+-----+-----+-----+-----+-----+-----+------+----+-----+-----+-----+-----+-----+-----+-----+
 *
 * RX = R27:R26
 * RY = R29:R28
 * RZ = R31:R30
 *
 * AVR Data Memory Map
 *
 * +======================+
 * |     Data Memory      | 0x0000 - 0x08FF
 * +======================+
 * |     32 Registers     | 0x0000 - 0x001F
 * +----------------------+
 * |   64 IO Registers    | 0x0020 - 0x005F
 * +----------------------+
 * | 160 Ext IO Registers | 0x0060 - 0x00FF
 * +----------------------+
 * |     Interal SRAM     | 0x0100 - 0x08FF
 * +----------------------+
 */
typedef struct AVR_MCU {
    /** @brief Program counter. */
    uint16_t pc;

    /** @brief Status Register. */
    uint8_t *sreg;

    /** @brief Stack pointer. */
    uint16_t *sp;

    /** @brief Working registers offset in data memory. */
    uint8_t *reg;

    /** @brief IO registers offset in data memory. */
    uint8_t *io_reg;

    /** @brief Extended IO registers offset in data memory. */
    uint8_t *ext_io_reg;

    /** @brief Internal SRAM offset in data memory. */
    uint8_t *sram;

    /** @brief Entire data memory used by other members. */
    uint8_t data[AVR_MCU_DATA_SIZE];

    /** @brief Flash memory. */
    uint16_t flash[AVR_MCU_FLASH_SIZE / sizeof(uint16_t)];

    /** @brief EEPROM memory. */
    uint8_t eeprom[AVR_MCU_EEPROM_SIZE];
} AVR_MCU;

/**
 * @brief Initialize the memory inside of MCU, MUST be called on creatation.
 *
 * - Zeros memory
 * - Sets pointers to proper offsets
 * - Sets stack pointer to RAMEND
 *
 * @param mcu Microcontroller Emulator
 */
void avr_mcu_init(AVR_MCU *restrict mcu);

/**
 * @brief Program the MCU with a AVR hex file compiled using arduino-cli.
 *
 * @param mcu Microcontroller Emulator
 * @param hex Hex instruction file
 * @return AVR_OK on success
 */
AVR_Result avr_program(AVR_MCU *restrict mcu, const char *restrict hex);

/**
 * @brief Cycle the CPU, fetching and executing one instruction from programmed flash.
 *
 * @param mcu Microcontroller Emulator
 */
void avr_cycle(AVR_MCU *restrict mcu);

#endif // _AVR__AVR_H_
