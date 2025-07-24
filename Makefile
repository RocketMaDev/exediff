TARGET := exediff

SRC_DIR := ./src
INC_DIR := ./include
BUILD_DIR := ./build

C_SRCS := $(shell find $(SRC_DIR) ! -name 'main.c' -name '*.c')
TEST_SRCS := $(shell find $(TEST_DIR) -name '*.c')

OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.c.o,$(C_SRCS))
TEST_OBJS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%.c.o,$(TEST_SRCS))

EXEDIFF := $(BUILD_DIR)/exediff.c.o

CC := gcc

CFLAGS := -fpie -fstack-protector -Wall -Wextra
LDFLAGS := -z now -z noexecstack -fpie -fstack-protector -Wall -Wextra -lcapstone -lelf -lkeystone

ifdef DEBUG
	ifeq ($(DEBUG),1)
		CFLAGS += -g -O2
		LDFLAGS += -g -O2
	else ifeq ($(DEBUG),2)
		CFLAGS += -g
		LDFLAGS += -g
	endif
else
	CFLAGS += -O2
	LDFLAGS += -O2
endif

all: exediff

exediff: $(OBJS) $(EXEDIFF)
	$(CC) $(LDFLAGS) $^ -o $@
	mv -f $@ $(BUILD_DIR)

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) -I$(INC_DIR) $(CFLAGS) $< -c -o $@
$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) -I$(INC_DIR) $(CFLAGS) $< -c -o $@
$(BUILD_DIR)/%.c.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	$(CC) -I$(INC_DIR) $(CFLAGS) $< -c -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR) $(BUILD_UTIL) $(BUILD_TEST)

.PHONY: clean all
clean:
	rm -rf $(BUILD_DIR)
