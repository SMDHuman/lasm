//-----------------------------------------------------------------------------
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdint.h>
#include <stdio.h>

#define HH_DARRAY_IMPLEMENTATION
#include "hh_darray.h"

//-----------------------------------------------------------------------------
#define MAX_TOKEN_SIZE 255
#define ERR 255

//-----------------------------------------------------------------------------
typedef enum{
	NONE,
	WORD,
	NUMBER,	
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
	BSLASH,
	ASTERISK,
	QUEST,
	EXCLA,
	DOT,
}TOKEN_ID;

typedef struct{
	uint16_t id;
	uint32_t line;
	uint32_t col;
	char filename[32];	
	char text[MAX_TOKEN_SIZE];
}token_t;

uint8_t tokenize(FILE* file, char *filename, hh_darray_t* tokens);
uint8_t is_alpha(char c);
uint8_t is_inside(char c, const char* chars);

//-----------------------------------------------------------------------------
#ifdef TOKENIZER_IMPLEMENTATION
//-----------------------------------------------------------------------------

uint8_t tokenize(FILE* file, char *filename, hh_darray_t* tokens){
	uint32_t line = 1, col = 1;
	uint8_t comment = 0;
	char word[MAX_TOKEN_SIZE];
	token_t token = {0};
	strcat(token.filename, filename);
	while(1){
		char cr = fgetc(file);col++;
		// Leave on end of file
		if(cr == EOF) break;
		//...
		if(cr == '\n'){
			line++;
			col = 0;
		}
		// Ignore comments
		if(cr == '\n' && comment == 2) comment = 0;
		if(cr != '\n' && comment == 2) continue;
		if(cr == '/' && comment == 0) {comment = 1; continue;}
		if(cr == '/' && comment == 1) {comment = 2; continue;}
		// Check chars
		token.id = NONE;
		memset(token.text, 0 ,MAX_TOKEN_SIZE);
		if(cr == '<') token.id=MACRO_O;
		if(cr == '>'){
			token_t t1; hh_darray_get(tokens, hh_darray_get_item_fill(tokens)-1, &t1);
			token_t t2; hh_darray_get(tokens, hh_darray_get_item_fill(tokens)-2, &t2);
			if(t1.id == WORD && t2.id == MACRO_O){
				hh_darray_popend(tokens, 0);
				hh_darray_popend(tokens, 0);
				t1.id = MACRO_ARG;
				hh_darray_append(tokens, &t1);
			}else {
				token.id=MACRO_C;
			}
		}
		if(cr == '(') token.id=RBRAC_O;
		if(cr == ')') token.id=RBRAC_C;
		if(cr == '[') token.id=SBRAC_O;
		if(cr == ']') token.id=SBRAC_C;
		if(cr == '{') token.id=CBRAC_O;
		if(cr == '}') token.id=CBRAC_C;
		if(cr == '#') token.id=HASH;
		if(cr == ':') token.id=COLON;
		if(cr == '+') token.id=PLUS;
		if(cr == '-') token.id=MINUS;
		if(cr == '/') token.id=SLASH;
		if(cr == '\\') token.id=BSLASH;
		if(cr == '*') token.id=ASTERISK;
		if(cr == '?') token.id=QUEST;
		if(cr == '!') token.id=EXCLA;
		if(cr == '\n' || cr == ';'){
			token_t t; hh_darray_get(tokens, hh_darray_get_item_fill(tokens)-1, &t);
			if(t.id != NEWLINE){
				token.id=NEWLINE; 
				cr = ';';
			}
		}
		if(token.id != NONE){
			token.line=line;
			token.col=col-1;
			token.text[0] = cr;
			hh_darray_append(tokens, &token);			
		}
		// Parse numbers
		if(is_inside(cr, "1234567890")){
			memset(word, 0, MAX_TOKEN_SIZE);
			while(is_inside(cr, "1234567890xabcdef")){
				strcat(word, &cr);
				cr = fgetc(file);
				col++;
			}
			token.id=NUMBER;
			token.line=line;
			token.col=col-strlen(word);
			memcpy(token.text, word, MAX_TOKEN_SIZE);
			hh_darray_append(tokens, &token);
			fseek(file, -1, SEEK_CUR);
		}
		// Parse words
		if(is_alpha(cr) || cr == '_'){
			memset(word, 0, MAX_TOKEN_SIZE);
			while(is_alpha(cr) || cr == '_' || cr == '.'){
				strcat(word, &cr);
				cr = fgetc(file);
				col++;
			}
			token.id=WORD;
			token.line=line;
			token.col=col-strlen(word);
			memcpy(token.text, word, MAX_TOKEN_SIZE);
			hh_darray_append(tokens, &token);
			fseek(file, -1, SEEK_CUR);
		}
		// Parse double qoute string
		if(cr == '"'){
			memset(word, 0, MAX_TOKEN_SIZE);
			while(1){
				cr = fgetc(file);
				if(cr == '"') break;
				strcat(word, &cr);
				col++;
			}
			token.id=STRING_DB;
			token.line=line;
			token.col=col-strlen(word);
			memcpy(token.text, word, MAX_TOKEN_SIZE);
			hh_darray_append(tokens, &token);
		}
		// Parse Single qoute string
		if(cr == '\''){
			memset(word, 0, MAX_TOKEN_SIZE);
			while(1){
				cr = fgetc(file);
				if(cr == '\'') break;
				strcat(word, &cr);
				col++;
			}
			token.id=STRING_SG;
			token.line=line;
			token.col=col-strlen(word);
			memcpy(token.text, word, MAX_TOKEN_SIZE);
			hh_darray_append(tokens, &token);
		}
	}
	return 0;
}
//-----------------------------------------------------------------------------
uint8_t is_alpha(char c){
	if(((uint8_t)c <= 90 && (uint8_t)c >= 65) || 
	((uint8_t)c <= 122 && (uint8_t)c >= 97)) return 1;
	return 0;
}
//-----------------------------------------------------------------------------
uint8_t is_inside(char c, const char* chars){
	for(uint16_t i = 0; chars[i] != 0; i++){
		if(c == chars[i]) return c;
	}
	return 0;
}

#endif
#endif
