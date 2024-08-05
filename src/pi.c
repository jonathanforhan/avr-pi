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
 * === PINOUT ===
 * AVR  GPIO  INO
 * PB0  0     8
 * PB1  1     9
 * PB2  2     10
 * PB3  3     11
 * PB4  4     12
 * PB5  5     13
 * PB6  6     -
 * PB7  7     -
 * PC0  8     A0
 * PC1  9     A1
 * PC2  10    A2
 * PC3  11    A3
 * PC4  12    A4
 * PC5  13    A5
 * PC6  14    -
 * -    -     -
 * PD0  16    0
 * PD1  17    1
 * PD2  18    2
 * PD3  19    3
 * PD4  20    4
 * PD5  21    5
 * PD6  22    6
 * PD7  23    7
 */

#include <fcntl.h>
#include <pigpio.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <avr.h>
#include "avr_defs.h"
#include "defs.h"

#define VERSION  "0.0.0"
#define MAX_PATH 260
#define LOG_NAME "avr-pi.log"

// can only be used when times are within +/- 1sec (our clk period will never be that long)
#define diff_timespec(T0, T1) (1000000000L * ((T1).tv_sec - (T0).tv_sec) + ((T1).tv_nsec - (T0).tv_nsec))

// error range / 2
#define ERR_RANGE_2 4

static AVR_MCU mcu;
static volatile sig_atomic_t sigint = 0;

// want to avoid as many syscalls as possible so state is cached
static uint8_t ddrb_state, portb_state; // b
static uint8_t ddrc_state, portc_state; // c
static uint8_t ddrd_state, portd_state; // d

static void signal_handler(int sig) {
    sigint = sig;
}

static void print_version(void) {
    printf("avr-pi v%s\n", VERSION);
}

static void print_help(void) {
    printf(
        "avr-pi usage:\n"
        "\tavr-pi --version \tGet avr-pi version info.\n"
        "\tavr-pi --help    \tGet avr-pi help.\n"
        "\tavr-pi {file}.hex\tExecute a compiled AVR hex file.\n");
}

// update ddr on GPIO
// reg: AVR reg
// state: global state ptr
// offset: offset of GPIO DCM
// return: true if state changed
static inline bool update_ddr(uint8_t reg, uint8_t *state, uint8_t offset) {
    if (mcu.data[reg] != *state) {
        for (int i = 0; i < 8; i++) {
            if (GET_BIT(mcu.data[reg], i) != GET_BIT(*state, i)) {
                gpioSetMode(i + offset, GET_BIT(mcu.data[reg], i) ? PI_OUTPUT : PI_INPUT);
            }
        }
        *state = mcu.data[reg];
        return true;
    }
    return false;
}

// update pin in AVR
// reg: AVR reg
// ddr: AVR ddr
// offset: offset of GPIO DCM
static inline void update_pin(uint8_t reg, uint8_t ddr, uint8_t offset) {
    for (int i = 0; i < 8; i++) {
        if (GET_BIT(mcu.data[ddr], i) == 0) {
            SET_BIT(mcu.data[reg], i, gpioRead(i + offset));
        }
    }
}

// update port in AVR
// reg: AVR reg
// ddr: AVR ddr
// offset: offset of GPIO DCM
static inline void update_port(uint8_t reg, uint8_t ddr, uint8_t *state, uint8_t offset) {
    if (mcu.data[reg] != *state) {
        for (int i = 0; i < 8; i++) {
            if (GET_BIT(mcu.data[ddr], i) == 1) {
                gpioWrite(i + offset, GET_BIT(mcu.data[reg], i));
            }
        }
        *state = mcu.data[reg];
    }
}

static inline void setup(void) {
    for (int i = 0; i < 8; i++) {
        gpioSetMode(i + 0, GET_BIT(mcu.data[REG_DDRB], i) ? PI_OUTPUT : PI_INPUT);
        gpioSetMode(i + 8, GET_BIT(mcu.data[REG_DDRC], i) ? PI_OUTPUT : PI_INPUT);
        gpioSetMode(i + 16, GET_BIT(mcu.data[REG_DDRD], i) ? PI_OUTPUT : PI_INPUT);

        if (GET_BIT(mcu.data[REG_DDRB], i)) {
            gpioWrite(i + 0, GET_BIT(mcu.data[REG_PORTB], i));
        } else {
            SET_BIT(mcu.data[REG_PINB], i, gpioRead(i + 0));
        }

        if (GET_BIT(mcu.data[REG_DDRC], i)) {
            gpioWrite(i + 8, GET_BIT(mcu.data[REG_PORTC], i));
        } else {
            SET_BIT(mcu.data[REG_PINB], i, gpioRead(i + 8));
        }

        if (GET_BIT(mcu.data[REG_DDRD], i)) {
            gpioWrite(i + 16, GET_BIT(mcu.data[REG_PORTD], i));
        } else {
            SET_BIT(mcu.data[REG_PIND], i, gpioRead(i + 16));
        }
    }

    ddrb_state  = mcu.data[REG_DDRB];
    ddrc_state  = mcu.data[REG_DDRC];
    ddrd_state  = mcu.data[REG_DDRD];
    portb_state = mcu.data[REG_PORTB];
    portc_state = mcu.data[REG_PORTC];
    portd_state = mcu.data[REG_PORTD];
}

static inline void loop(void) {
    struct timespec t0, t1;
    int cycles;
    long err = 0; // measures accumulation of error each iteration

    while (!sigint) {
        (void)clock_gettime(CLOCK_MONOTONIC, &t0);

        cycles = avr_execute(&mcu);

        // spend approximately one clk period on each cycle
        // errors are tracked and accounted for
        while (cycles) {
            do {
                (void)clock_gettime(CLOCK_MONOTONIC, &t1);
            } while (diff_timespec(t0, t1) - (AVR_MCU_CLK_PERIOD - err) < ERR_RANGE_2);
            err = diff_timespec(t0, t1) - (AVR_MCU_CLK_PERIOD - err);

            avr_cycle(&mcu);

            cycles += avr_interrupt(&mcu);

            cycles--;
            if (cycles) {
                (void)clock_gettime(CLOCK_MONOTONIC, &t0);
            }
        }

        update_ddr(REG_DDRB, &ddrb_state, 0);
        update_ddr(REG_DDRC, &ddrc_state, 8);
        update_ddr(REG_DDRD, &ddrd_state, 16);

        // TODO optimize, too slow
        // update_pin(REG_PINB, REG_DDRB, 0);
        // update_pin(REG_PINC, REG_DDRC, 8);
        // update_pin(REG_PIND, REG_DDRD, 16);

        update_port(REG_PORTB, REG_DDRB, &portb_state, 0);
        update_port(REG_PORTC, REG_DDRC, &portc_state, 8);
        update_port(REG_PORTD, REG_DDRD, &portd_state, 16);
    }
}

int main(int argc, char *argv[]) {
    char *buf = NULL;
    int fd    = -1;
    struct stat st;

    if (argc < 2) {
        goto error;
    } else if (strncmp(argv[1], "--help", 6) == 0) {
        print_help();
        return 0;
    } else if (strncmp(argv[1], "--version", 9) == 0) {
        print_version();
        return 0;
    } else if (strnlen(argv[1], MAX_PATH) <= 4 || strcasecmp(strrchr(argv[1], '.'), ".hex") != 0) {
        LOG_ERROR("invalid hex file");
        goto error;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        LOG_ERROR("could not read file %s", argv[1]);
        goto error;
    }

    if (fstat(fd, &st) < 0) {
        LOG_ERROR("could not read input file size");
        goto error;
    }

    buf = malloc(st.st_size + 1); // +1 NULL byte
    if (buf == NULL) {
        LOG_ERROR("allocation failure");
        goto error;
    }

    switch (read(fd, buf, st.st_size)) {
    case -1:
        LOG_ERROR("could not read file %s", argv[1]);
        goto error;
    case 0:
        LOG_ERROR("file is empty %s", argv[1]);
        goto error;
    default:
        buf[st.st_size] = '\0';
    }

    close(fd);
    fd = -1;

    // logging goes to filesystem (DEBUG BUILD ONLY)
    assert((stderr = fopen(LOG_NAME, "w"))); // NOLINT

    avr_mcu_init(&mcu);

    if (avr_program(&mcu, buf) != AVR_OK) {
        LOG_ERROR("failed to write program to flash");
        goto error;
    }

    free(buf);
    buf = NULL;

    if (gpioInitialise() == PI_INIT_FAILED) {
        LOG_ERROR("failed to initialize GPIO interface");
        goto error;
    } else {
        if (signal(SIGINT, signal_handler) != SIG_ERR) {
            setup();
            loop();
        }
        for (int i = 0; i < 24; i++) {
            gpioSetMode(i, PI_INPUT); // cleanup
        }
        gpioTerminate();
    }

    return 0;

error:
    close(fd);
    free(buf);
    print_help();
    return -1;
}
