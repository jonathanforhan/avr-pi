#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "mcu.h"

static char buf[MB];

int main(int argc, char* argv[]) {
    int fd  = -1;
    mcu mcu = {0};

    if (argc < 2 || strlen(argv[1]) < 5 || strcasecmp(strrchr(argv[1], '.'), ".hex")) {
        LOG_ERROR("invalid command");
        goto err;
    }

    if ((fd = open(argv[1], O_RDONLY)) < 0) {
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

    if (program(&mcu, buf) != OK) {
        LOG_ERROR("failed to write program to flash");
        goto err;
    }

    cycle(&mcu);

    return 0;

err:
    close(fd);

    fprintf(stderr,
            "\navr-pi usage:\n"
            "\tavr-pi {file}.hex\n");

    exit(-1);
}
