# Compiler and flags
CC := cc
CFLAGS := -ggdb -Wall -Wextra -I include


std6502: std6502.c build
	$(CC) -o build/$@ $< $(CFLAGS)

build:
	mkdir -p build

clean:
	rm -f build/*

all: clean std6502