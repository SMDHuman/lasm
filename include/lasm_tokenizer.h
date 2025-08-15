//-----------------------------------------------------------------------------
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM_TOKENIZER_H
#define LASM_TOKENIZER_H

#include <stdint.h>
#include <stdio.h>

#include "hh_darray.h"

//-----------------------------------------------------------------------------
#define MAX_TOKEN_SIZE 255
#define ERR 255

//-----------------------------------------------------------------------------
typedef enum{
	NONE,
	WORD,
	NUMBER,	
	VECTOR,
	SIZE,
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
	char filename[32];	
	char text[MAX_TOKEN_SIZE];
}token_t;

uint8_t lasm_tokenize(FILE* file, char *filename, hh_darray_t* tokens);
uint8_t is_lineend_token_id(hh_darray_t *tokens, uint32_t index, TOKEN_ID id);
uint8_t is_lineend_token_text(hh_darray_t *tokens, uint32_t index, const char* text);
char char_upper(char c);
char char_lower(char c);
uint8_t is_alpha(char c);
uint8_t is_inside(char c, const char* chars);
void print_error_loc(token_t *token);
void print_tokens_as_code(hh_darray_t* tokens);
const char* token_id_to_string(TOKEN_ID id);

//-----------------------------------------------------------------------------
#ifdef LASM_TOKENIZER_IMPLEMENTATION
//-----------------------------------------------------------------------------

uint8_t lasm_tokenize(FILE* file, char *filename, hh_darray_t* tokens){
	uint32_t line = 1, col = 1;
	uint8_t comment = 0;
	char word[MAX_TOKEN_SIZE];
	token_t token = {0};
	strcat(token.filename, filename);
	while(1){
		char cr = fgetc(file);
		// Leave on end of file
		if(cr == EOF) break;
		//...
		if(cr == '\n'){
			line++;
			col = 1;
		}
		// Ignore comments
		if(cr == '\n' && comment == 2) comment = 0;
		if(cr != '\n' && comment == 2) {col++; continue;}
		if(cr == '/' && comment == 0) {comment = 1; col++; continue;}
		if(cr == '/' && comment == 1) {comment = 2; col++;continue;}
		// Check chars
		token.id = NONE;
		memset(token.text, 0 ,MAX_TOKEN_SIZE);
		if(cr == '<'){
						char next = fgetc(file);
						if(next == '<'){
							token.id = BITSHIFT_L;
						}else{
							fseek(file, -1, SEEK_CUR);
							token.id=MACRO_O;
						}
		}
		if(cr == '>'){
						char next = fgetc(file);
						if(next == '>'){
							token.id = BITSHIFT_R;
						}else{
							fseek(file, -1, SEEK_CUR);
							token_t t1; hh_darray_get(tokens, hh_darray_get_item_fill(tokens)-1, &t1);
							token_t t2; hh_darray_get(tokens, hh_darray_get_item_fill(tokens)-2, &t2);
							if(t1.id == WORD && t2.id == MACRO_O){
								hh_darray_popend(tokens, 0);
								hh_darray_popend(tokens, 0);
								t1.id = MACRO_ARG;
								hh_darray_append(tokens, &t1);
								col++;
							}else {
								token.id=MACRO_C;
							}
						}
					}

		if(cr == '(') token.id=RBRAC_O;
		if(cr == ')') token.id=RBRAC_C;
		if(cr == '[') token.id=SBRAC_O;
		if(cr == ']') token.id=SBRAC_C;/*{
						token_t t1; hh_darray_get(tokens, hh_darray_get_item_fill(tokens)-1, &t1);
						token_t t2; hh_darray_get(tokens, hh_darray_get_item_fill(tokens)-2, &t2);
						if(t1.id == NUMBER && t2.id == SBRAC_O){
							hh_darray_popend(tokens, 0);
							hh_darray_popend(tokens, 0);
							t1.id = VECTOR;
							hh_darray_append(tokens, &t1);
							col++;
						}else {
							token.id=SBRAC_C;
						}
					}*/
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
		if(cr == '.') token.id=DOT;
		if(cr == ',') token.id=COMMA;
		if(cr == '\n' || cr == ';'){
			token_t t; hh_darray_get(tokens, hh_darray_get_item_fill(tokens)-1, &t);
			if(t.id != NEWLINE){
				token.id=NEWLINE; 
				cr = ';';
			}
		}
		if(token.id != NONE){
			col++;
			token.line=line;
			token.col=col-1;
			token.text[0] = cr;
			if(token.id == BITSHIFT_L || token.id == BITSHIFT_R) token.text[1] = token.text[0];
			hh_darray_append(tokens, &token);			
		}
		// Parse numbers
		if(is_inside(cr, "1234567890")){
			memset(word, 0, MAX_TOKEN_SIZE);
			while(is_inside(char_lower(cr), "1234567890xabcdef")){
				strcat(word, &cr);
				cr = fgetc(file);
				col++;
			}
			token.id=NUMBER;
			token.line=line;
			token.col=col-strlen(word);
			memcpy(token.text, word, MAX_TOKEN_SIZE);
			{
				token_t t1; hh_darray_get(tokens, hh_darray_get_item_fill(tokens)-1, &t1);
				if(t1.id == DOT){
					hh_darray_popend(tokens, 0);
					token.id = SIZE;
				}
			}
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
// Checks if the token before first newline equal to given id
uint8_t is_lineend_token_id(hh_darray_t *tokens, uint32_t index, TOKEN_ID id){
  token_t *token = hh_darray_get_reference(tokens, index);
  uint32_t i;
  for(i = index + 1; token->id != NEWLINE; i++){
    token = hh_darray_get_reference(tokens, i);
  }
  token = hh_darray_get_reference(tokens, i-2);
  return token->id == id ? 1 : 0;
}
//-----------------------------------------------------------------------------
uint8_t is_lineend_token_text(hh_darray_t *tokens, uint32_t index, const char* text){
  token_t *token = hh_darray_get_reference(tokens, index);
  uint32_t i;
  for(i = index + 1; token->id != NEWLINE; i++){
    token = hh_darray_get_reference(tokens, i);
  }
  token = hh_darray_get_reference(tokens, i-2);
  return strcmp(token->text, text) == 0 ? 1 : 0;
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
//-----------------------------------------------------------------------------
char char_upper(char c){
	if((uint8_t)c <= 122 && (uint8_t)c >= 97) return (char)((uint8_t)c - 32);
	return c;
}
//-----------------------------------------------------------------------------
char char_lower(char c){
	if((uint8_t)c <= 90 && (uint8_t)c >= 65) return (char)((uint8_t)c + 32);
	return c;
}
//-----------------------------------------------------------------------------
void print_error_loc(token_t *token){
	printf("[ERROR] '%s':%d:%d: ", token->filename, token->line, token->col);
}
void print_tokens_as_code(hh_darray_t* tokens){
	token_t token;
	uint32_t head = 0;
	const uint32_t tokens_end = hh_darray_get_item_fill(tokens);
	while(head < tokens_end){
		hh_darray_get(tokens, head++, &token);
		if(token.id == NEWLINE) printf(";\n");
		else if(token.id == STRING_DB) printf("\"%s\"", token.text);
		else if(token.id == STRING_SG) printf("'%s'", token.text);
		else if(token.id == VECTOR) printf("[%s]", token.text);
		else if(token.id == SIZE) printf(".%s", token.text);
		else printf("%s ", token.text);
	}		
}
const char* token_id_to_string(TOKEN_ID id){
	switch(id){
		case NONE: return "NONE";
		case WORD: return "WORD";
		case NUMBER: return "NUMBER";
		case VECTOR: return "VECTOR";
		case SIZE: return "SIZE";
		case STRING_DB: return "STRING_DB";
		case STRING_SG: return "STRING_SG";
		case RBRAC_O: return "RBRAC_O";
		case RBRAC_C: return "RBRAC_C";
		case CBRAC_O: return "CBRAC_O";
		case CBRAC_C: return "CBRAC_C";
		case SBRAC_O: return "SBRAC_O";
		case SBRAC_C: return "SBRAC_C";
		case MACRO_O: return "MACRO_O";
		case MACRO_C: return "MACRO_C";
		case MACRO_ARG: return "MACRO_ARG";
		case MACRO_INCLUDE: return "MACRO_INCLUDE";
		case NEWLINE: return "NEWLINE";
		case HASH: return "HASH";
		case COLON: return "COLON";
		case PLUS: return "PLUS";
		case MINUS: return "MINUS";
		case SLASH: return "SLASH";
		case BITSHIFT_L: return "BITSHIFT_L";
		case BITSHIFT_R: return "BITSHIFT_R";
		case BSLASH: return "BSLASH";
		case ASTERISK: return "ASTERISK";
		case QUEST: return "QUEST";
		case EXCLA: return "EXCLA";
		case DOT: return "DOT";
		case COMMA: return "COMMA";
		case INDEX: return "INDEX";
		default: return "UNKNOWN";
	}
}

#endif
#endif
