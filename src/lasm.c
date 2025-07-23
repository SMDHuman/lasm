#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define HH_DARRAY_IMPLEMENTATION
#include "../include/hh_darray.h"
#define TOKENIZER_IMPLEMENTATION
#include "../include/tokenizer.h"
#define PARSER_IMPLEMENTATION
#include "../include/parser.h"
#include "cpu/6502.c"

//-----------------------------------------------------------------------------
uint8_t get_arg_index(int argc, char *argv[], const char word[]);
hh_darray_t tokens;
hh_darray_t lasm_vars;

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]){
	//...
	if(argc < 2){
		printf("[ERROR] no input file\n");
		return 0;
	}
	// Parse Arguments
	hh_darray_t include_paths;
	hh_darray_init(&include_paths, sizeof(size_t));
	FILE *file = fopen(argv[1], "r");
	if(file == NULL){
		printf("[ERROR] No file found named '%s'\n", argv[1]);
		return 0;
	}
	char *path = extract_folder_path(argv[1]);
	hh_darray_append(&include_paths, &path);
	
	// Tokenize input file
	hh_darray_init(&tokens, sizeof(token_t));
	if(tokenize(file, argv[1], &tokens) == ERR) return 0;
	// Find and apply includes
	if(find_apply_includes(&tokens, &include_paths) == ERR) return 0;
	// Preprocess the macros
	hh_darray_t macros; hh_darray_init(&macros, sizeof(hh_darray_t));
	if(preprocess_macros(&tokens, &macros) == ERR) return 0;
	newline_after_branches(&tokens);
	clean_newlines(&tokens);
	// Parse output file name if there is any
	uint8_t out_i = get_arg_index(argc, argv, "-o"); // Output file name
	char output_name[MAX_TOKEN_SIZE] = {0};
	if(out_i) strcat(output_name, argv[out_i+1]);
	else strcat(output_name, "a.out");
	// Parse selected cpu and assemble tokens
	uint8_t cpu_i = get_arg_index(argc, argv, "-m"); // Machine cpu name
	FILE *outf = fopen(output_name, "w");
	hh_darray_init(&lasm_vars, sizeof(lasm_var_t));
	if(cpu_i){
		if(strcmp(argv[cpu_i+1], "6502") == 0){
			printf("Assembling for 6502...\n");
			assemble_6502(&tokens, outf);
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
	fclose(outf);
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
