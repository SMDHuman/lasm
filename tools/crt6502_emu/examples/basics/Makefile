a.out: Makefile crt6502_example.l ../../crt_utils.l ../../../utilities/math.l 
	~/.local/bin/lasm crt6502_example.l -o a.out -m 6502 -i ../../../utilities/ -i ../../

run: a.out ../../build/crt6502
	../../build/crt6502 a.out

../../build/crt6502:
	make -C ../../..

clean: 
	rm -f a.out