# Compiler and flags
CC := cc
CFLAGS := -ggdb -Wall -Wextra -I include

std6502: std6502.c
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -f std6502

all: clean std6502