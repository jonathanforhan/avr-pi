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

#ifndef __AVR_H__
#define __AVR_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
    uint32_t pc;

    /** @brief Stack pointer. */
    uint32_t sp;

    /** @brief Status Register. */
    uint8_t sreg;

    /** @brief Working registers. */
    uint8_t reg[32];

    /** @brief IO registers. */
    uint8_t io_reg[64];

    /** @brief Extended IO registers. */
    uint8_t ext_io_reg[160];

    /** @brief SRAM data section, 2KB. */
    uint8_t data[0x0800];

    /** @brief Flash memory, 32KB. */
    uint16_t flash[0x4000];

    /** @brief EEPROM memory, 1KB. */
    uint8_t eeprom[0x0400];
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

#warning "disable always test before release"
#define AVR_TEST

#ifdef AVR_TEST
/**
 * @brief Run build tests.
 *
 * @return AVR_OK on success
 */
AVR_Result avr_run_tests(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // __AVR_H__
