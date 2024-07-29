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

#ifndef __AVR_H__
#define __AVR_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
 * @def AVR_MCU_STRAM_OFFSET
 * @brief Data offset used by sram.
 */
#define AVR_MCU_SRAM_OFFSET 0x0100

/**
 * @def AVR_MCU_DATA_SIZE
 * @brief Size of data section in MCU, 2304 bytes.
 */
#define AVR_MCU_DATA_SIZE 0x08FF

/**
 * @def AVR_MCU_FLASH_SIZE
 * @brief Size of flash section in MCU, 32KB.
 */
#define AVR_MCU_FLASH_SIZE 0x8000

/**
 * @def AVR_MCU_EEPROM_SIZE
 * @brief Size of eeprom section in MCU, 1KB.
 */
#define AVR_MCU_EEPROM_SIZE 0x08FF

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
 */
typedef struct AVR_MCU {
    /** @brief Program counter. */
    uint16_t pc;

    /** @brief Stack pointer. */
    uint16_t sp;

    /** @brief Status Register. */
    uint8_t sreg;

    /** @brief Working registers offset in data memory. */
    uint8_t *reg;

    /** @brief IO registers offset in data memory. */
    uint8_t *io_reg;

    /** @brief Extended IO registers offset in data memory. */
    uint8_t *ext_io_reg;

    /** @brief SRAM data section offset in data memory. */
    uint8_t *sram;

    /**
     * @brief Entire data memory used by other members.
     *
     * +----------------------+
     * |     Data Memory      | 0x0000 - 0x08FF
     * +----------------------+
     * |     32 Registers     | 0x0000 - 0x001F
     * +----------------------+
     * |   64 IO Registers    | 0x0020 - 0x005F
     * +----------------------+
     * | 160 Ext IO Registers | 0x0060 - 0x00FF
     * +----------------------+
     * |     Interal SRAM     | 0x0100 - 0x08FF
     * +----------------------+
     */
    uint8_t data[AVR_MCU_DATA_SIZE];

    /** @brief Flash memory. */
    uint16_t flash[AVR_MCU_FLASH_SIZE / sizeof(uint16_t)];

    /** @brief EEPROM memory. */
    uint8_t eeprom[AVR_MCU_EEPROM_SIZE];
} AVR_MCU;

/**
 * @brief Initialize the memory inside of mcu, MUST be called on creatation.
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

#endif // __AVR_H__
