#!/bin/bash

# compile falgs used by arduino
# -g -Os -w -fpermissive -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -Wno-error=narrowing -mmcu=atmega328p -DF_CPU=16000000L -DARDUINO=10607 -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR

BOARD="arduino:avr:uno"

for dir in `ls -d $(dirname -- "$0")/*/ | xargs -n 1 | sed 's/.$//'`; do
    arduino-cli compile --quiet -b "$BOARD" "$dir/$(basename $dir).ino" --output-dir "$dir" &
done

wait
