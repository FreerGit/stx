
CC = gcc
CFLAGS = -std=c2x -Wextra -Wall -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 -Wwrite-strings  -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -fno-omit-frame-pointer -fno-var-tracking-assignments -Wformat=2
SANFLAGS = -fsanitize=address -fsanitize=undefined -fsanitize-address-use-after-scope
TARGET := a.out
BUILD_FOLDER = build

SRCS := $(wildcard test/*.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

all: $(TARGET)
$(TARGET): $(OBJS)
	@$(CC) $(CFLAGS) $(SANFLAGS) -o $@ $(BUILD_FOLDER)/$(notdir $^)
%.o: %.c
	@$(CC) $(CFLAGS) $(SANFLAGS)  -c $< -o $(BUILD_FOLDER)/$(notdir $@)

test: $(TARGET)
	@echo " "
	@./$(TARGET)

clean:
	rm -rf $(TARGET) *.o
	
.PHONY: all clean test

