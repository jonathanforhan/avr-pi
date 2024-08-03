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
 *
 * AVR Interrupts
 *
 * The first 26 words of flash contain interrupt vectors
 *
 * Vec #  Addr     Source         Interrupt Description
 * +----+--------+--------------+-------------------------------
 * | 1  | 0x0000 | RESET        | External pin, power-on reset, brown-out reset and watchdog system reset
 * | 2  | 0x0002 | INT0         | External interrupt request 0
 * | 3  | 0x0004 | INT1         | External interrupt request 1
 * | 4  | 0x0006 | PCINT0       | Pin change interrupt request 0
 * | 5  | 0x0008 | PCINT1       | Pin change interrupt request 1
 * | 6  | 0x000A | PCINT2       | Pin change interrupt request 2
 * | 7  | 0x000C | WDT          | Watchdog time-out interrupt
 * | 8  | 0x000E | TIMER2 COMPA | Timer/Counter2 compare match A
 * | 9  | 0x0010 | TIMER2 COMPB | Timer/Counter2 compare match B
 * | 10 | 0x0012 | TIMER2 OVF   | Timer/Counter2 overflow
 * | 11 | 0x0014 | TIMER1 CAPT  | Timer/Counter1 capture event
 * | 12 | 0x0016 | TIMER1 COMPA | Timer/Counter1 compare match A
 * | 13 | 0x0018 | TIMER1 COMPB | Timer/Counter1 compare match B
 * | 14 | 0x001A | TIMER1 OVF   | Timer/Counter1 overflow
 * | 15 | 0x001C | TIMER0 COMPA | Timer/Counter0 compare match A
 * | 16 | 0x001E | TIMER0 COMPB | Timer/Counter0 compare match B
 * | 17 | 0x0020 | TIMER0 OVF   | Timer/Counter0 overflow
 * | 18 | 0x0022 | SPI, STC     | SPI serial transfer complete
 * | 19 | 0x0024 | USART, RX    | USART Rx complete
 * | 20 | 0x0026 | USART, UDRE  | USART, data register empty
 * | 21 | 0x0028 | USART, TX    | USART, Tx complete
 * | 22 | 0x002A | ADC          | ADC conversion complete
 * | 23 | 0x002C | EE READY     | EEPROM ready
 * | 24 | 0x002E | ANALOG COMP  | Analog comparator
 * | 25 | 0x0030 | TWI          | 2-wire serial interface
 * | 26 | 0x0032 | SPM READY    | Store program memory ready
 */

#ifndef _AVR__AVR_H_
#define _AVR__AVR_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def AVR_MCU_CLK_SPEED
 * @brief Clock speed of 16MHz
 */
#define AVR_MCU_CLK_SPEED 16320000L

/**
 * @def AVR_MCU_CLK_PERIOD
 * @brief Clock period in nanoseconds
 */
#define AVR_MCU_CLK_PERIOD (1000000000L / AVR_MCU_CLK_SPEED)

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
#define AVR_MCU_DATA_SIZE 0x0900

/**
 * @def AVR_MCU_FLASH_SIZE
 * @brief Size of MCU flash section in bytes, 32KB.
 */
#define AVR_MCU_FLASH_SIZE 0x8000

/**
 * @def AVR_MCU_EEPROM_SIZE
 * @brief Size of eeprom section in MCU, 1KB.
 */
#define AVR_MCU_EEPROM_SIZE 0x0400

/**
 * @def AVR_MCU_RAMEND
 * @brief End of all SRAM (registers, io, and internal).
 */
#define AVR_MCU_RAMEND (AVR_MCU_DATA_SIZE - 1)

/**
 * @brief Result type for avr_* functions.
 *
 * @copydoc avr.h
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
    /** @brief Idle mode enabled. */
    bool idle;

    /** @brief System clock. */
    uint16_t clk;

    /** @brief Program counter. */
    uint_fast16_t pc;

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
 * @brief Execute one instruction one instruction from programmed flash.
 *
 * Execute does not cycle the CPU clock, instead it returns the number of cycles an operation took,
 * this is becuase special timing must be taken into account.
 *
 * @param mcu Microcontroller Emulator
 * @return Number of cycles taken during execution
 */
int avr_execute(AVR_MCU *restrict mcu);

/**
 * @brief Check for interrupts and possibly trigger.
 *
 * Interrupt does not cycle the CPU clock, instead it returns the number of cycles an operation took,
 * if an interrupt did not trigger this will be zero.
 *
 * @note MUST call avr_interrupt AFTER avr_execute because of how it stores PC on ISR
 *
 * @param mcu Microcontroller Emulator
 * @return Number of cycles taken during interrupt
 */
int avr_interrupt(AVR_MCU *restrict mcu);

/**
 * @brief Cycle the CPU clock one time.
 *
 * @param mcu Microcontroller Emulator
 */
void avr_cycle(AVR_MCU *restrict mcu);

#ifdef __cplusplus
}
#endif

#endif // _AVR__AVR_H_
