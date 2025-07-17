#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define HH_DARRAY_IMPLEMENTATION
#include "../include/hh_darray.h"
#define TOKENIZER_IMPLEMENTATION
#include "../include/tokenizer.h"
#include "cpu/6502.c"

//-----------------------------------------------------------------------------
uint8_t get_arg_index(int argc, char *argv[], const char word[]);
uint8_t preprocess_macros(hh_darray_t *tokens);

hh_darray_t tokens;

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]){
	//...
	if(argc < 2){
		printf("[ERROR] no input file\n");
		return 0;
	}
	//...
	FILE *file = fopen(argv[1], "r");
	if(file == NULL){
		printf("[ERROR] No file found named '%s'\n", argv[1]);
		return 0;
	}
	// Tokenize input file
	hh_darray_init(&tokens, sizeof(token_t));
	if(tokenize(file, argv[1], &tokens) == ERR) return 0;
	// Preprocess the macros
	if(preprocess_macros(&tokens) == ERR) return 0;
	//...
	printf("number of tokens: %ld\n", hh_darray_get_item_fill(&tokens));
	// Parse output file name if there is any
	uint8_t out_i = get_arg_index(argc, argv, "-o"); // Output file name
	char output_name[MAX_TOKEN_SIZE] = {0};
	if(out_i) strcat(output_name, argv[out_i+1]);
	else strcat(output_name, "a.out");
	// Parse selected cpu and assemble tokens
	uint8_t cpu_i = get_arg_index(argc, argv, "-m"); // Machine cpu name
	if(cpu_i){
		if(strcmp(argv[cpu_i+1], "6502") == 0){
			printf("Assembling for 6502...\n");
			assemble_6502(&tokens, output_name);
		}else{
			printf("[ERROR] Machine named '%s' not found\n", argv[cpu_i+1]);
			return 0;
		}
	}else{
		printf("[ERROR] No machine specified\n");
		return 0;
	}
	//...
	printf("Done!\n");
	hh_darray_deinit(&tokens);
	return 0;
}

//-----------------------------------------------------------------------------

uint8_t preprocess_macros(hh_darray_t *tokens){
	// Extract macros
	// Tokenize includes
	// preprocess includes
	// merge includes macros with main
	// Apply define and function macros to all macros 
	// Apply define and function macros main tokens 
	return 0;
}

//-----------------------------------------------------------------------------
uint8_t get_arg_index(int argc, char *argv[], const char word[]){
	for(uint8_t i = 0; i < argc-1; i++){
		if(strcmp(argv[i], word) == 0){
			return(i);
		}
	}
	return 0;
}
