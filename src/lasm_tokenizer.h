//-----------------------------------------------------------------------------
// lasm_tokenizer.h
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM_TOKENIZER_H
#define LASM_TOKENIZER_H

#include <stdint.h>
#include <stdio.h>

#include "hh_darray.h"

//-----------------------------------------------------------------------------
#define MAX_TOKEN_SIZE 64
#define ERR 255

//-----------------------------------------------------------------------------
typedef enum{
	NONE,
	WORD,
	NUMBER,	
	VECTOR,
	STRING_DB,
	STRING_SG,
	RBRAC_O,
	RBRAC_C,
	CBRAC_O,
	CBRAC_C,
	SBRAC_O,
	SBRAC_C,
	MACRO_O,
	MACRO_C,
	MACRO_ARG,
	MACRO_INCLUDE,
	NEWLINE,
	HASH,
	COLON,
	PLUS,
	MINUS,
	SLASH,
	BITSHIFT_L,
	BITSHIFT_R,
	BSLASH,
	ASTERISK,
	QUEST,
	EXCLA,
	DOT,
	COMMA,
	INDEX,
}TOKEN_ID;

typedef struct{
	uint16_t id;
	uint32_t line;
	uint32_t col;
	char filename[34];	
	char text[MAX_TOKEN_SIZE];
}token_t;

uint8_t lasm_tokenize(FILE* file, char *filename, hh_darray_t* tokens);
char char_upper(char c);
char char_lower(char c);
uint8_t is_alpha(char c);
uint8_t is_alphanum(char c);
uint8_t is_inside(char c, const char* chars);
void print_error_loc(token_t *token);
void print_tokens_as_code(hh_darray_t* tokens);
const char* token_id_to_string(TOKEN_ID id);

#endif
