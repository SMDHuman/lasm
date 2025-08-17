//-----------------------------------------------------------------------------
// lasm_macro.c
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "lasm_macro.h"

//-----------------------------------------------------------------------------
uint8_t lasm_find_apply_includes(hh_darray_t *tokens, hh_darray_t *include_paths){
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
					if(lasm_tokenize(file, token.text, &include_tokens) == ERR) return ERR;
					// Find and apply includes in it
					if(lasm_find_apply_includes(&include_tokens, include_paths) == ERR) return 0;
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
uint8_t lasm_extract_macros(hh_darray_t *tokens, hh_darray_t *macros){
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
uint8_t lasm_apply_macros(hh_darray_t *tokens, hh_darray_t *macros){
	for(uint32_t i = 0; i < hh_darray_get_item_fill(tokens); i++){
		token_t *token = hh_darray_get_reference(tokens, i);
		if(token->id == WORD || token->id == MACRO_ARG){
			hh_darray_t macro_tokens;
			hh_darray_t macro_arg_macros; hh_darray_init(&macro_arg_macros, sizeof(hh_darray_t)); 
			uint8_t store_arguments = 1;
			uint32_t push_count = 0;
			if(lasm_find_and_get_macro(token, macros, &macro_tokens)){
				// Apply macro tokens
				hh_darray_pop(tokens, i, 0); // Consume macro name
				for(uint32_t j = 1; j < hh_darray_get_item_fill(&macro_tokens); j++){
					token_t macro_token; hh_darray_get(&macro_tokens, j, &macro_token);
					// Store macro agruments
					if(macro_token.id == MACRO_ARG && store_arguments){
						hh_darray_append(&macro_arg_macros, 0);
						hh_darray_t *new_macro_arg_tokens = hh_darray_get_end_reference(&macro_arg_macros);
						hh_darray_init(new_macro_arg_tokens, sizeof(token_t));
						hh_darray_append(new_macro_arg_tokens, &macro_token);
						while(token->id != COMMA && token->id != NEWLINE){
							hh_darray_append(new_macro_arg_tokens, token);
							hh_darray_pop(tokens, i, 0);
						}
						hh_darray_pop(tokens, i, 0); // Consume comma token
					}else{
						store_arguments = 0;
						hh_darray_t macro_arg_tokens;
						if(lasm_find_and_get_macro(&macro_token, &macro_arg_macros, &macro_arg_tokens)){
							// Apply macro argument tokens
							for(uint32_t k = 1; k < hh_darray_get_item_fill(&macro_arg_tokens); k++){
								token_t arg_token; hh_darray_get(&macro_arg_tokens, k, &arg_token);
								hh_darray_push(tokens, i + push_count++, &arg_token);
							}
						}else{
							hh_darray_push(tokens, i + push_count++, &macro_token);
						}
					}
				}
				hh_darray_deinit(&macro_arg_macros);
				i--;
			}
		}
	}
	return 0;
}
//-----------------------------------------------------------------------------
uint8_t lasm_find_and_get_macro(token_t *token, hh_darray_t *macros, hh_darray_t *macro_tokens){
	for(uint32_t i = 0; i < hh_darray_get_item_fill(macros); i++){
		hh_darray_t macro; hh_darray_get(macros, i, &macro);
		token_t m_label; hh_darray_get(&macro, 0, &m_label);
		if(m_label.id == token->id){
			if(strcmp(m_label.text, token->text) == 0){
				// Copy macro tokens
				memcpy(macro_tokens, &macro, sizeof(hh_darray_t));
				return 1;
			}
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
uint8_t lasm_clear_multi_newlines(hh_darray_t *tokens){
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
	return 0;
}
//-----------------------------------------------------------------------------
uint8_t lasm_newline_after_branches(hh_darray_t *tokens){
	for(uint32_t i = 0; i < hh_darray_get_item_fill(tokens); i++){
		token_t token; hh_darray_get(tokens, i, &token);
		if(token.id == COLON){
			token_t token_nl; memcpy(&token_nl, &token, sizeof(token_nl));
			token_nl.text[0] = ';'; token_nl.id = NEWLINE;
			hh_darray_push(tokens, ++i, &token_nl);
		}					
	}	
	return 0;
}