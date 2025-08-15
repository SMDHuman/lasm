INCLUDE := -I include/ -I src/cpu/
OBJECTS := build/lasm_namespace.o build/lasm_assembler.o build/lasm_parser.o

build/lasm: build src/lasm.c include/* src/cpu/*  $(OBJECTS)
	gcc -o build/lasm $(INCLUDE) $(OBJECTS) src/lasm.c 

build/lasm_namespace.o: include/lasm_namespace.*
	gcc -c include/lasm_namespace.c -o build/lasm_namespace.o $(INCLUDE)

build/lasm_assembler.o: include/lasm_assembler.*
	gcc -c include/lasm_assembler.c -o build/lasm_assembler.o $(INCLUDE)

build/lasm_parser.o: include/lasm_parser.*
	gcc -c include/lasm_parser.c -o build/lasm_parser.o $(INCLUDE)

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