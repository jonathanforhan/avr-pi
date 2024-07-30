CLI := arduino-cli
BOARD := arduino:avr:uno

SKETCHES := $(wildcard *)

.PHONY: all clean

all: $(SKETCHES)
	$(CLI) compile --quiet -b $(BOARD) $< --output-dir $(shell pwd)

clean:
	$(RM) *.bin
	$(RM) *.eep
	$(RM) *.elf
	$(RM) *.hex
