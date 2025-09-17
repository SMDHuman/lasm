# Compiler and flags
CC := cc
CFLAGS := -ggdb -Wall -Wextra -I include -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

build/crt6502: crt6502.c build
	$(CC) -o $@ $< $(CFLAGS)

build:
	mkdir -p build

clean:
	rm -f build/*

all: clean build/crt6502