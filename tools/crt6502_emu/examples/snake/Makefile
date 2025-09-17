snake.out: Makefile snake.l ../../crt_utils.l ../../../utilities/math.l 
	~/.local/bin/lasm snake.l -o snake.out -m 6502 -i ../../../utilities/ -i ../../

run: snake.out ../../build/crt6502
	../../build/crt6502 snake.out

../../build/crt6502:
	make -C ../../..

clean: 
	rm -f snake.out