#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define HH_DARRAY_IMPLEMENTATION
#include "../include/hh_darray.h"

#define ERR 255

uint8_t tokenize(FILE* file, hh_darray_t* tokens);
uint8_t is_alpha(char c);
uint8_t is_inside(char c, const char* chars);

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
	NEWLINE,
	HASH,
	COLON,
	PLUS,
	MINUS,
	SLASH,
	BSLASH,
	ASTERISK,
}TOKEN_ID;

#define MAX_TOKEN_SIZE 255

typedef struct{
	uint16_t id;
	uint32_t line;
	uint32_t col;
	char text[MAX_TOKEN_SIZE];
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

	printf("number of tokens: %ld\n", hh_darray_get_item_fill(&tokens));
	for(uint32_t i = 0; i < hh_darray_get_item_fill(&tokens); i++){
		token_t token;
		hh_darray_get(&tokens, i, &token);
		if(token.id == NEWLINE) printf("\n");
		else if(token.id == STRING_DB) printf("\"%s\" ", token.text);
		else if(token.id == STRING_SG) printf("\'%s\' ", token.text);
		else printf("%s ", token.text);
		//printf("id: %d, line: %d, col: %d, text: '%s'\n", 
		//		token.id, token.line, token.col, token.text);
	}

	hh_darray_deinit(&tokens);
	return 0;
}

uint8_t tokenize(FILE* file, hh_darray_t* tokens){
	uint32_t line = 1, col = 1;
	uint8_t comment = 0;
	char word[MAX_TOKEN_SIZE];
	token_t token;
	while(1){
		char cr = fgetc(file);col++;
		// Leave on end of file
		if(cr == EOF) break;
		// Ignore comments
		if(cr == '\n' && comment == 2) comment = 0;
		if(cr != '\n' && comment == 2) continue;
		if(cr == '/' && comment == 0) {comment = 1; continue;}
		if(cr == '/' && comment == 1) {comment = 2; continue;}
		// Check chars
		token.id = NONE;
		memset(token.text, 0 ,MAX_TOKEN_SIZE);
		if(cr == '<') token.id=MACRO_O;
		if(cr == '>') token.id=MACRO_C;
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
			while(is_alpha(cr) || cr == '_'){
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
		//...
		if(cr == '\n'){
			line++;
			col = 0;
		}
	}
	return 0;
}

uint8_t is_alpha(char c){
	if(((uint8_t)c <= 90 && (uint8_t)c >= 65) || 
	((uint8_t)c <= 122 && (uint8_t)c >= 97)) return 1;
	return 0;
}

uint8_t is_inside(char c, const char* chars){
	for(uint16_t i = 0; chars[i] != 0; i++){
		if(c == chars[i]) return c;
	}
	return 0;
}
