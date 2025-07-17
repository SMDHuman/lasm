build/lasm: build/ src/lasm.c include/* src/cpu/*
	gcc -o build/lasm src/lasm.c -I include -I src/cpu

build/:
	mkdir build

clear:
	rm build -r -f
