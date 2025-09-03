//-----------------------------------------------------------------------------
// lasm_macro.c
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "lasm_macro.h"

static const char TAG[] = "MCRO";

//-----------------------------------------------------------------------------
uint8_t lasm_find_apply_includes(hh_darray_t *tokens, hh_darray_t *include_paths){
	uint8_t inside_macro = 0;
	uint32_t macro_size = 0;
	token_t token;
	for(uint32_t i = 0; i < hh_darray_get_item_fill(tokens); i++){
		hh_darray_get(tokens, i, &token);
		//...
		if(token.id == MACRO_C){
			inside_macro--;
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
						free(path_merge);
						if(file != NULL) break;
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
					hh_darray_deinit(&include_tokens);
				}
			}
			if(inside_macro == 0) macro_size = 0;
		}

		if(inside_macro > 0) macro_size++;
		if(token.id == MACRO_O) inside_macro++;
	}	
	
	return 0;
}

//-----------------------------------------------------------------------------
uint8_t lasm_extract_macros(hh_darray_t *tokens, hh_darray_t *macros){
	int8_t inside_macro = 0;
	int8_t inside_macro_args = 0;
	macro_t* macro;
	token_t* token;
	for(uint32_t i = 0; i < hh_darray_get_item_fill(tokens); i++){
		token = hh_darray_get_reference(tokens, i);
		//...
		if(inside_macro > 0){
			if(token->id == RBRAC_O && hh_darray_get_item_fill(&macro->tokens) == 0){
				hh_darray_pop(tokens, i, 0); // Consume '('
				inside_macro_args++;
			}else if (inside_macro_args > 0){
				printf("token: %s\n", token->text);
				hh_darray_append(&macro->args, token);
				hh_darray_pop(tokens, i, 0); // Consume argument name
				if(token->id == COMMA){
					hh_darray_pop(tokens, i, 0); // Consume ','
				}else if(token->id == RBRAC_C){
					hh_darray_pop(tokens, i, 0); // Consume ')'
					inside_macro_args--;
				}else{
					printf(TAG);
					print_error_loc(token);
					printf("Unexpected token found while parsing macro arguments '%s'\n", token->text);
					return ERR;
				}
			}else if(token->id == MACRO_C && inside_macro == 1){
				inside_macro--;
				hh_darray_pop(tokens, i, 0); // Consume '>'
				if(lasm_extract_macros(&macro->tokens, macros) == ERR) return ERR;
			}else{
				if(token->id == MACRO_O) inside_macro++;
				if(token->id == MACRO_C) inside_macro--;
				hh_darray_append(&macro->tokens, token);
				hh_darray_pop(tokens, i, 0);
			}
			i--;
		}
		//...
		else if(token->id == MACRO_O){
			hh_darray_pop(tokens, i, 0); // Consume '<'
			if(token->id == EXCLA || token->id == QUEST){
				uint8_t qe_select = (token->id == QUEST);
				hh_darray_pop(tokens, i, 0); // Consume '!'
				uint8_t found = (uint8_t)(lasm_find_and_get_macro(token, macros) != NULL);
				if(qe_select) found = !found;
				hh_darray_pop(tokens, i, 0); // Consume name
				inside_macro ++;
				size_t old_i = i;
				// skip removing macro
				while(inside_macro){
					token = hh_darray_get_reference(tokens, i);
					if(found) hh_darray_pop(tokens, i, 0);
					else i++;
					if(token->id == MACRO_O) inside_macro++;
					if(token->id == MACRO_C) inside_macro--;
				}
				if(!found) i--;
				hh_darray_pop(tokens, i, 0); // Consume '<'
				if(!found) i = old_i;
			}else{
				hh_darray_append(macros, 0);
				macro = hh_darray_get_end_reference(macros);
				macro->name = *token;
				hh_darray_pop(tokens, i--, 0); // Consume name
				hh_darray_init(&macro->tokens, sizeof(token_t));
				hh_darray_init(&macro->args, sizeof(token_t));
				inside_macro++;
			}
		}
	}
	if(inside_macro > 0){
		printf(TAG);
		print_error_loc(&macro->name);
		printf("Can't find any macro closer\n");
		return ERR;
	}
	return 0;
}
//-----------------------------------------------------------------------------
uint8_t lasm_apply_macros(hh_darray_t *tokens, hh_darray_t *macros){
	for(uint32_t i = 0; i < hh_darray_get_item_fill(tokens); i++){
		token_t* token = hh_darray_get_reference(tokens, i);
		if(token->id == WORD){
			macro_t* macro = lasm_find_and_get_macro(token, macros);
			if(macro){
				// Apply macro
				printf("Applying macro '%s'\n", macro->name.text);
				hh_darray_pop(tokens, i, 0); // Consume macro name
				if(hh_darray_get_item_fill(&macro->args) > 0){
					if(token->id != RBRAC_O){
						print_error_loc(token);
						printf("Macro '%s' requires arguments\n", macro->name.text);
						return ERR;
					}
					hh_darray_pop(tokens, i, 0); // Consume '('
					// store arguments as macros
					for(size_t j = 0; j < hh_darray_get_item_fill(&macro->args); j++){
						hh_darray_append(macros, 0);
						macro_t* new_macro = hh_darray_get_end_reference(macros);
						hh_darray_init(&new_macro->tokens, sizeof(token_t));
						hh_darray_get(&macro->args, j, &new_macro->name);
						size_t ip = 0;
						size_t brace_level = 1;
						while(1){
							token_t* arg_token = hh_darray_get_reference(tokens, i + ip);
							printf("arg_token: %s for %s\n", arg_token->text, new_macro->name.text);
							hh_darray_pop(tokens, i + ip, 0); // Consume all until ')' or ','
							if(arg_token->id == RBRAC_C) brace_level--;
							if(arg_token->id == RBRAC_O) brace_level++;
							if(brace_level == 0 || arg_token->id == COMMA){
								break;
							}
							hh_darray_append(&new_macro->tokens, token);
							ip++;
						}
					}
				}
			}
		}
	}
	return 0;
}
//-----------------------------------------------------------------------------
macro_t* lasm_find_and_get_macro(token_t *token, hh_darray_t *macros){
	for(uint32_t i = 0; i < hh_darray_get_item_fill(macros); i++){
		macro_t *macro = hh_darray_get_reference(macros, i);
		if(macro->name.id == token->id){
			if(strcmp(macro->name.text, token->text) == 0){
				return macro;
			}
		}
	}
	return NULL;
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
		if(token.id == COLON || token.id == CBRAC_O){
			token_t token_nl; memcpy(&token_nl, &token, sizeof(token_nl));
			token_nl.text[0] = ';'; token_nl.id = NEWLINE;
			hh_darray_push(tokens, ++i, &token_nl);
		}else if(token.id == CBRAC_C){
			token_t token_nl; memcpy(&token_nl, &token, sizeof(token_nl));
			token_nl.text[0] = ';'; token_nl.id = NEWLINE;
			hh_darray_push(tokens, i++, &token_nl);
		}
	}
	return 0;
}
//-----------------------------------------------------------------------------
void print_macros(hh_darray_t *macros){
	for(uint32_t i = 0; i < hh_darray_get_item_fill(macros); i++){
		macro_t *macro = hh_darray_get_reference(macros, i);
		printf("Macro %d: %s\n", i, macro->name.text);
		for(uint32_t j = 0; j < hh_darray_get_item_fill(&macro->args); j++){
			token_t *arg = hh_darray_get_reference(&macro->args, j);
			printf("  Arg %d: %s\n", j, arg->text);
		}
		for(uint32_t j = 0; j < hh_darray_get_item_fill(&macro->tokens); j++){
			token_t *token = hh_darray_get_reference(&macro->tokens, j);
			printf("  Token %d: %s\n", j, token->text);
		}
	}
}
