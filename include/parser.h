//-----------------------------------------------------------------------------
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include "hh_darray.h"
#include "tokenizer.h"
//-----------------------------------------------------------------------------
typedef struct{
	uint32_t value;
	uint32_t address;
	uint32_t tokens_origin;
	char *name;
	token_t *token;
}lasm_var_t;

typedef struct{
	uint32_t head;
	uint32_t address;
	hh_darray_t *tokens;
}token_reader_t;

extern hh_darray_t lasm_vars; // sizeof(lasm_var_t)
extern token_reader_t lasm_tokens;

//-----------------------------------------------------------------------------
uint8_t parse(hh_darray_t *tokens, uint32_t *head, uint32_t *addres);
uint8_t eval_expression(hh_darray_t *tokens, uint32_t index);
uint8_t get_var(char *name, lasm_var_t *lasm_var);
uint8_t expect_token_name(char *name);
uint8_t expect_token_id(TOKEN_ID id);
//...
uint8_t preprocess_macros(hh_darray_t *tokens, hh_darray_t *macros);
uint8_t find_apply_includes(hh_darray_t *tokens, hh_darray_t *include_paths);
uint8_t extract_macros(hh_darray_t *tokens, hh_darray_t *macros);
uint8_t apply_macros(hh_darray_t *tokens, hh_darray_t *macros);
void clean_newlines(hh_darray_t *tokens);
void newline_after_branches(hh_darray_t *tokens);
void print_macros(hh_darray_t *macros);
void print_error_loc(token_t *token);
char* extract_folder_path(const char* path);

//-----------------------------------------------------------------------------
#ifdef PARSER_IMPLEMENTATION
//-----------------------------------------------------------------------------
uint8_t parse(hh_darray_t *tokens, uint32_t *head, uint32_t *addres){
	token_t token; hh_darray_get(tokens, *head, &token);
	if(token.id == WORD){
		lasm_var_t var;
		if(get_var(token.text, &var)){
			
		}else{
			
		}
	}
}

//-----------------------------------------------------------------------------
uint8_t get_var(char *name, lasm_var_t *lasm_var){
	for(uint32_t i = 0; i < hh_darray_get_item_fill(&lasm_vars); i++){
		lasm_var_t var; hh_darray_get(&lasm_vars, i, &var);
		if(strcmp(name, var.name) == 0){
			if(lasm_var) memcpy(lasm_var, &var, sizeof(lasm_var_t));
			return 1;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
uint8_t preprocess_macros(hh_darray_t *tokens, hh_darray_t *macros){
	// Extract macros
	if(extract_macros(tokens, macros) == ERR) return ERR;
	//print_macros(macros);
	// Apply macros main tokens
	if(apply_macros(tokens, macros) == ERR) return ERR;
	return 0;
}

//-----------------------------------------------------------------------------
uint8_t find_apply_includes(hh_darray_t *tokens, hh_darray_t *include_paths){
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
					FILE *file;
					for (uint16_t i = 0; i < hh_darray_get_item_fill(include_paths); i++){
						char *path; hh_darray_get(include_paths, i, &path);
						size_t size = strlen(token.text) + strlen(path) + 2;
						char *path_merge = malloc(size); memset(path_merge, 0, size);
						strcat(path_merge, path);
						strcat(path_merge, "/");
						strcat(path_merge, token.text);	
						file = fopen(path_merge, "r");
						if(file != NULL) break;
						free(path_merge);
					}
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
					if(find_apply_includes(&include_tokens, include_paths) == ERR) return 0;
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
	int8_t macro_inside = 0;
	hh_darray_t *macro_tokens;
	token_t opener_token;
	for(uint32_t i = 0; i < hh_darray_get_item_fill(tokens); i++){
		token_t token; hh_darray_get(tokens, i, &token);
		//...
		if(token.id == MACRO_C){
			macro_inside--;
			if(macro_inside < 0){
				print_error_loc(&token);
				printf("Too much macro closer\n");
				return ERR;
			}
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
				memcpy(&opener_token, &token, sizeof(token_t));
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
	if(macro_inside > 0){
		print_error_loc(&opener_token);
		printf("Can't find any macro closer\n");
		return ERR;
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
void clean_newlines(hh_darray_t *tokens){
	// Clean up unnecessary newlines
	uint8_t prev_nl = 1;
	for(uint32_t i = 0; i < hh_darray_get_item_fill(tokens); i++){
		token_t t1; hh_darray_get(tokens, i, &t1);
		if(t1.id == NEWLINE){
			if(prev_nl) hh_darray_pop(tokens, i--, 0);
			else prev_nl = 1;
		}else{
			prev_nl = 0;
		}
	}
}
//-----------------------------------------------------------------------------
void newline_after_branches(hh_darray_t *tokens){
	for(uint32_t i = 0; i < hh_darray_get_item_fill(tokens); i++){
		token_t token; hh_darray_get(tokens, i, &token);
		if(token.id == COLON){
			token_t token_nl; memcpy(&token_nl, &token, sizeof(token_nl));
			token_nl.text[0] = ';'; token_nl.id = NEWLINE;
			hh_darray_push(tokens, ++i, &token_nl);
		}					
	}	
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
char* extract_folder_path(const char* path){
	uint16_t size = 0;
	for(uint16_t i = 0; path[i] != 0; i++){
		if(path[i] == '/' || path[i] == '\\') size = i;
	}
	char* folder_path = malloc(size+1);
	memcpy(folder_path, path, size);
	folder_path[size] = 0;
	return(folder_path);
}

//-----------------------------------------------------------------------------
void print_error_loc(token_t *token){
	printf("[ERROR] '%s':%d:%d:", token->filename, token->line, token->col);
}


#endif
#endif
