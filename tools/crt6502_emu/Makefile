# Compiler and flags
CC := cc
CFLAGS := -ggdb -Wall -Wextra -I include -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

ctr6502: ctr6502.c build
	$(CC) -o build/$@ $< $(CFLAGS)

build:
	mkdir -p build

clean:
	rm -f build/*

all: clean ctr6502