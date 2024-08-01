#!/bin/bash

BOARD="arduino:avr:uno"

for dir in `ls -d $(dirname -- "$0")/*/ | xargs -n 1 | sed 's/.$//'`; do
    arduino-cli compile --quiet -b "$BOARD" "$dir/$(basename $dir).ino" --output-dir "$dir"
done
