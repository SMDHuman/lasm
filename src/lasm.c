#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define HH_DARRAY_IMPLEMENTATION
#include "../include/hh_darray.h"

#define ERR 255

uint8_t tokenize(FILE* file, hh_darray_t* tokens);
uint8_t is_inside(char c, const char* chars);

typedef enum{
	NONE,
	WORD,
	NUMBER,	
	MATH,   // +, -, /, *
	RBRAC_O,
	RBRAC_C,
	CBRAC_O,
	CBRAC_C,
	SBRAC_O,
	SBRAC_C,
	NEWLINE,
	SPECL,
}TOKEN_ID;

typedef struct{
	uint16_t id;
	uint32_t line;
	uint32_t col;
	char *text;
}token_t;

hh_darray_t tokens;

int main(int argc, char *argv[]){
	if(argc < 2){
		printf("[ERROR] no input file\n");
		return 0;
	}

	FILE *file = fopen(argv[1], "r");
	if(file == NULL){
		printf("[ERROR] No file found named '%s'\n", argv[1]);
		return 0;
	}

	hh_darray_init(&tokens, sizeof(token_t));
	if(tokenize(file, &tokens) == ERR) return 0;

	hh_darray_deinit(&tokens);
	return 0;
}

uint8_t tokenize(FILE* file, hh_darray_t* tokens){
	uint32_t line = 1, col = 1;
	uint8_t comment = 0;
	while(1){
		char cr = fgetc(file);
		// Leave on end of file
		if(cr == EOF) break;
		// Ignore comments
		if(cr == '\n' && comment == 2) comment = 0;
		if(cr != '\n' && comment == 2) continue;
		if(cr == '/' && comment == 0) {comment = 1; continue;}
		if(cr == '/' && comment == 1) {comment = 2; continue;}
		//...
		if(cr == '\n'){
			line++;
			col = 0;
		}col++;
	}
	return 0;
}

uint8_t is_inside(char c, const char* chars){
	for(uint16_t i = 0; chars[i] != 0; i++){
		if(c == chars[i]) return c;
	}
	return 0;
}
