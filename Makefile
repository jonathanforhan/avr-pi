TARGET := avr-pi

BUILD_DIR := build
SRC_DIR := src

CFLAGS := -std=gnu99 -MMD -MP -Wall -Wextra -Wpedantic


DEBUG ?= 1

ifeq ($(DEBUG), 1)
	CFLAGS += -g3
	OUT_DIR := $(BUILD_DIR)/debug
else
	CFLAGS += -O3 -DNDEBUG
	OUT_DIR := $(BUILD_DIR)/release
endif


INCS := $(shell find $(SRC_DIR) -type d)
SRCS := $(shell find $(SRC_DIR) -name '*.c' -or -name '*.s')
OBJS := $(SRCS:%=$(OUT_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CFLAGS += $(addprefix -I,$(INCS))


$(OUT_DIR)/$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OUT_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)/*


-include $(DEPS)
