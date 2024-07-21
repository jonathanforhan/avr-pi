EXE := avr-pi

CFLAGS := $(INC_FLAGS) -MMD -MP -Wall -Wextra -Wpedantic -Werror

BUILD_DIR = build

DEBUG ?= 1

ifeq ($(DEBUG), 1)
	CFLAGS += -g3 -DNDEBUG
	OUT_DIR := build/debug
else
	CFLAGS += -O3
	OUT_DIR := build/release
endif

SRC_DIR := src

SRCS := $(shell find $(SRC_DIR) -name '*.c' -or -name '*.s')
OBJS := $(SRCS:%=$(OUT_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIR) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

$(OUT_DIR)/$(EXE): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OUT_DIR)/%.c.o: %.c
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)/*


-include $(DEPS)
