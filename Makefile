build/lasm: build/ src/lasm.c
	gcc -o build/lasm src/lasm.c

build/:
	mkdir build

clear:
	rm build -r -f
