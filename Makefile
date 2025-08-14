build/lasm: build src/lasm.c include/* src/cpu/*  build/lasm_namespace.o build/lasm_assembler.o
	gcc src/lasm.c build/lasm_namespace.o build/lasm_assembler.o -o build/lasm -I include -I src/cpu

build/lasm_namespace.o: include/lasm_namespace.c
	gcc -c include/lasm_namespace.c -o build/lasm_namespace.o -I include -I src/cpu

build/lasm_assembler.o: include/lasm_assembler.c
	gcc -c include/lasm_assembler.c -o build/lasm_assembler.o -I include -I src/cpu

build:
	mkdir build

clear:
	rm -r build

example_basics: build/lasm examples/basic_syntax.l
	./build/lasm examples/basic_syntax.l -m 6502 -o build/a.out

example_expressions: build/lasm examples/expressions.l
	./build/lasm examples/expressions.l -m 6502 -o build/a.out

example_namespaces: build/lasm examples/namespaces.l
	./build/lasm examples/namespaces.l -m 6502 -o build/a.out

example_fibonacci: build/lasm examples/fibonacci.l
	./build/lasm examples/fibonacci.l -m 6502 -o build/a.out

example_6502_addressing: build/lasm examples/6502_addressing_modes.l
	./build/lasm examples/6502_addressing_modes.l -m 6502 -o build/a.out	