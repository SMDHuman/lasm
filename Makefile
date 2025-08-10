build/lasm: build src/lasm.c include/* src/cpu/*  build/*.o
	gcc src/lasm.c build/*.o -o build/lasm -I include -I src/cpu 

build/lasm_parser.o: include/lasm_parser.c
	gcc -c include/lasm_parser.c -o build/lasm_parser.o -I include -I src/cpu

build:
	mkdir build

clear:
	rm build -r -f

example_basics: build/lasm examples/basic_syntax.l
	./build/lasm examples/basic_syntax.l -m 6502 -o build/a.out

example_expressions: build/lasm examples/expressions.l
	./build/lasm examples/expressions.l -m 6502 -o build/a.out
example_namespaces: build/lasm examples/namespaces.l
	./build/lasm examples/namespaces.l -m 6502 -o build/a.out
