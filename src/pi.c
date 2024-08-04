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

#include <fcntl.h>
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

static inline void run(void) {
    struct timespec t0, t1;
    int cycles;
    long err = 0; // measures accumulation of error each iteration

    for (;;) {
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

    run();

    return 0;

error:
    close(fd);
    free(buf);
    print_help();
    return -1;
}
