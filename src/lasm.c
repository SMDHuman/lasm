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
uint8_t preprocess_macros(hh_darray_t *tokens, hh_darray_t *macros);
uint8_t find_apply_includes(hh_darray_t *tokens);
uint8_t extract_macros(hh_darray_t *tokens, hh_darray_t *macros);
uint8_t apply_macros(hh_darray_t *tokens, hh_darray_t *macros);
void print_error_loc(token_t *token);
void print_macros(hh_darray_t *macros);

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
	// Find and apply includes
	if(find_apply_includes(&tokens) == ERR) return ERR;
	// Preprocess the macros
	hh_darray_t macros; hh_darray_init(&macros, sizeof(hh_darray_t));
	if(preprocess_macros(&tokens, &macros) == ERR) return 0;
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
uint8_t preprocess_macros(hh_darray_t *tokens, hh_darray_t *macros){
	// Extract macros
	extract_macros(tokens, macros);
	print_macros(macros);
	// Apply macros main tokens
	apply_macros(tokens, macros);
	 
	return 0;
}

//-----------------------------------------------------------------------------
uint8_t find_apply_includes(hh_darray_t *tokens){
	uint8_t macro_inside = 0;
	uint32_t macro_size = 0;
	token_t token;
	for(uint32_t i = 0; i < hh_darray_get_item_fill(tokens); i++){
		hh_darray_get(tokens, i, &token);
		//...
		if(token.id == MACRO_C){
			macro_inside--;
			if(macro_size == 1){
				hh_darray_get(tokens, i-1, &token);
				if(token.id == STRING_DB){
					// Include
					//printf("INCLUDING: %s \n", token.text);
					FILE *file = fopen(token.text, "r");
					if(file == NULL){
						print_error_loc(&token);
						printf("No file found named '%s'\n", token.text);
						return ERR;
					}
					// Tokenize input file
					hh_darray_t include_tokens;
					hh_darray_init(&include_tokens, sizeof(token_t));
					if(tokenize(file, token.text, &include_tokens) == ERR) return ERR;
					// Find and apply includes in it
					if(find_apply_includes(&include_tokens) == ERR) return 0;
					hh_darray_pop(tokens, i-2, 0);
					hh_darray_pop(tokens, i-2, 0);
					hh_darray_pop(tokens, i-2, 0);
					for(uint32_t j = 0; j < hh_darray_get_item_fill(&include_tokens); j++){
						hh_darray_get(&include_tokens, j, &token);				
						hh_darray_push(tokens, i-2+j, &token);					
					}
				}
			}
			if(macro_inside == 0) macro_size = 0;
		}

		if(macro_inside) macro_size++;
		if(token.id == MACRO_O) macro_inside++;

	}	
	
	return 0;
}

//-----------------------------------------------------------------------------
uint8_t extract_macros(hh_darray_t *tokens, hh_darray_t *macros){
	uint8_t macro_inside = 0;
	hh_darray_t *macro_tokens;
	for(uint32_t i = 0; i < hh_darray_get_item_fill(tokens); i++){
		token_t token; hh_darray_get(tokens, i, &token);
		//...
		if(token.id == MACRO_C){
			macro_inside--;
			if(macro_inside == 0){
				// If there is any macro with same label, remove previus one
				for(uint32_t j = 0; j < hh_darray_get_item_fill(macros); j++){
					hh_darray_t macro; hh_darray_get(macros, j, &macro);
					token_t m_label; hh_darray_get(&macro, 0, &m_label);
					token_t n_label; hh_darray_get(macro_tokens, 0, &n_label);
					if(strcmp(m_label.text, n_label.text) == 0){
						hh_darray_pop(macros, j, 0);
					}
				}
				// Add new macro to macros array
				hh_darray_append(macros, macro_tokens);
				hh_darray_pop(tokens, i--, 0);
			}
		}
		//...
		if(macro_inside > 0){
			hh_darray_pop(tokens, i--, 0);
			hh_darray_append(macro_tokens, &token);
		}
		//...
		if(token.id == MACRO_O){
			// Gets in side new macro
			if(macro_inside == 0){
				// Conditional palcement macros
				hh_darray_get(tokens, i+1, &token);
				if(token.id == QUEST || token.id == EXCLA){
					uint8_t is_defined = 0;
					for(uint32_t j = 0; j < hh_darray_get_item_fill(macros); j++){
						hh_darray_t macro; hh_darray_get(macros, j, &macro);
						token_t m_label; hh_darray_get(&macro, 0, &m_label);
						token_t n_label; hh_darray_get(tokens, i+2, &n_label);
						if(strcmp(m_label.text, n_label.text) == 0){
							is_defined = 1;
							break;
						}
					}
					// Remove macro bracnets
					uint32_t start = i;
					hh_darray_pop(tokens, i, 0);
					hh_darray_pop(tokens, i, 0);
					hh_darray_pop(tokens, i, 0);
					if((is_defined && token.id==QUEST) || (!is_defined && token.id==EXCLA)){
						macro_inside++;
						while(1){
							hh_darray_get(tokens, i++, &token);
							if(token.id == MACRO_O) macro_inside++;
							if(token.id == MACRO_C) macro_inside--;
							if(macro_inside == 0){
								hh_darray_pop(tokens, i-1, 0);
								i = start;
								break;
							}
						}macro_inside--;
					}
					// Else just remove all
					else{
						macro_inside++;
						while(1){
							hh_darray_get(tokens, i, &token);
							if(token.id == MACRO_O) macro_inside++;
							if(token.id == MACRO_C) macro_inside--;
							hh_darray_pop(tokens, i, 0);
							if(macro_inside == 0){
								i = start;
								break;
							}
						}macro_inside--;
					}
				}else{
					macro_tokens = malloc(sizeof(hh_darray_t));
					hh_darray_init(macro_tokens, sizeof(token_t));
					hh_darray_pop(tokens, i--, 0);	
				}
			}
			macro_inside++;
		}
	}
	return 0;
}
//-----------------------------------------------------------------------------
uint8_t apply_macros(hh_darray_t *tokens, hh_darray_t *macros){
	for(uint32_t i = 0; i < hh_darray_get_item_fill(tokens); i++){
		token_t token; hh_darray_get(tokens, i, &token);
		if(token.id == WORD){
			for(uint32_t j = 0; j < hh_darray_get_item_fill(macros); j++){
				hh_darray_t macro_tokens;  hh_darray_get(macros, j, &macro_tokens);
				token_t label_token; hh_darray_get(&macro_tokens, 0, &label_token);
				if(strcmp(label_token.text, token.text) == 0){
					hh_darray_t macro_args; hh_darray_init(&macro_args, sizeof(token_t));
					hh_darray_t macro_args_c; hh_darray_init(&macro_args_c, sizeof(token_t));
					uint8_t take_arguments = 1;
					// Insert macro
					hh_darray_pop(tokens, i, 0); // remove label from main tokens
					for(uint32_t k = 0; k < hh_darray_get_item_fill(&macro_tokens)-1; k++){
						token_t m_token; hh_darray_get(&macro_tokens, k+1, &m_token);
						// Store arguments and contents for later use
						if(m_token.id == MACRO_ARG && take_arguments){
							hh_darray_append(&macro_args, &m_token);
							hh_darray_pop(tokens, i, &token);
							hh_darray_append(&macro_args_c, &token);
						}else{
							take_arguments = 0;
							int32_t is_arg = -1;
							for(uint32_t v = 0; v < hh_darray_get_item_fill(&macro_args); v++){
								token_t arg_token; hh_darray_get(&macro_args, v, &arg_token);
								if(strcmp(arg_token.text, m_token.text) == 0){
									is_arg = v;	
									break;
								}
							}
							if(is_arg == -1){
								hh_darray_push(tokens, i+k, &m_token);				
							}else{
								token_t arg_c_token; hh_darray_get(&macro_args_c, is_arg, &arg_c_token);
								hh_darray_push(tokens, i+k, &arg_c_token);												
							}
							
						}
					}
					hh_darray_deinit(&macro_args);
					hh_darray_deinit(&macro_args_c);
					i --;
					break;
				}
			}
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
void print_error_loc(token_t *token){
	printf("[ERROR] '%s':%d:%d:", token->filename, token->line, token->col);
}

//-----------------------------------------------------------------------------
void print_macros(hh_darray_t *macros){
	for(uint32_t i = 0; i < hh_darray_get_item_fill(macros); i++){
		hh_darray_t macro_tokens; hh_darray_get(macros, i, &macro_tokens);
		for(uint32_t j = 0; j < hh_darray_get_item_fill(&macro_tokens); j++){			
			token_t token; hh_darray_get(&macro_tokens, j, &token);
			printf("%s ", token.text);
		}		
		printf("\n");
	}
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
