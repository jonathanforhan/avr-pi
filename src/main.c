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
#include <unistd.h>

#include <avr.h>
#include "defs.h"

#define MAX_PATH 256

static char buf[512 * KB];

int main(int argc, char *argv[]) {
    int fd = -1;
    AVR_MCU mcu;

    avr_mcu_init(&mcu);

    if (argc < 2 || strnlen(argv[1], MAX_PATH) < 5 || strcasecmp(strrchr(argv[1], '.'), ".hex") != 0) {
        LOG_ERROR("invalid command");
        goto err;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        LOG_ERROR("invalid filepath %s", argv[1]);
        goto err;
    }

    switch (read(fd, buf, sizeof(buf))) {
    case -1:
        LOG_ERROR("could not read file %s", argv[1]);
        goto err;
    case 0:
        LOG_ERROR("file is empty %s", argv[1]);
        goto err;
    default:
        break;
    }

    buf[sizeof(buf) - 1] = 0;
    printf("%s", buf);

    if (avr_program(&mcu, buf) != AVR_OK) {
        LOG_ERROR("failed to write program to flash");
        goto err;
    }

    for (;;) {
        usleep(250000);
        avr_cycle(&mcu);
    }

    return 0;

err:
    close(fd);

    fprintf(stderr,
            "\navr-pi usage:\n"
            "\tavr-pi {file}.hex\n");

    exit(-1);
}
