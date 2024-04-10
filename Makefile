
CC = gcc
CFLAGS = -Wextra -Wall -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 -Wwrite-strings  -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -fsanitize=address -fsanitize=undefined -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fno-var-tracking-assignments -Wformat=2
