build/lasm: build src/lasm.c include/* src/cpu/*
	gcc -o build/lasm src/lasm.c -I include -I src/cpu

build:
	mkdir build

clear:
	rm build -r -f

example: build/lasm examples/basic_syntax.l
	./build/lasm examples/basic_syntax.l -m 6502 -o build/a.out
