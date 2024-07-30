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
#include <unistd.h>

#include <avr.h>
#include "defs.h"

#define VERSION "0.0.0"

#define MAX_PATH 256

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

int main(int argc, char *argv[]) {
    int fd = -1;
    struct stat st;
    char *buf = NULL;
    AVR_MCU mcu;

    if (argc < 2) {
        goto err;
    } else if (strncmp(argv[1], "--help", 6) == 0) {
        print_help();
        return 0;
    } else if (strncmp(argv[1], "--version", 9) == 0) {
        print_version();
        return 0;
    } else if (strnlen(argv[1], MAX_PATH) <= 4 || strcasecmp(strrchr(argv[1], '.'), ".hex") != 0) {
        LOG_ERROR("invalid hex file");
        goto err;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        LOG_ERROR("could not read file %s", argv[1]);
        goto err;
    }

    if (fstat(fd, &st) < 0) {
        LOG_ERROR("could not read file %s", argv[1]);
        goto err;
    }

    buf = malloc(st.st_size + 1); // +1 NULL byte
    if (buf == NULL) {
        LOG_ERROR("allocation failure");
        goto err;
    }

    switch (read(fd, buf, st.st_size)) {
    case -1:
        LOG_ERROR("could not read file %s", argv[1]);
        goto err;
    case 0:
        LOG_ERROR("file is empty %s", argv[1]);
        goto err;
    default:
        buf[st.st_size] = '\0';
    }

    close(fd);
    fd = -1;

    PRINT_DEBUG("%s\n", buf);

    avr_mcu_init(&mcu);

    if (avr_program(&mcu, buf) != AVR_OK) {
        LOG_ERROR("failed to write program to flash");
        goto err;
    }

    free(buf);
    buf = NULL;

    for (;;) {
        usleep(250000);
        avr_cycle(&mcu);
        PRINT_DEBUG("\t SREG: CZNVSHTI ");
        for (int i = 0; i < 8; i++) {
            PRINT_DEBUG("%u", GET_BIT(mcu.sreg, i));
        }
        PRINT_DEBUG("\n");
    }

    return 0;

err:
    close(fd);
    free(buf);
    print_help();
    return -1;
}
