#include <stdint.h>
#include <string.h>
#include "../../include/tokenizer.h"
#include "../../include/hh_darray.h"
#include "../../include/parser.h"

char char_upper(char c);
uint8_t is_instruction(token_t *token);

char inst_words[56][3]={"ADC","AND","ASL","BCC","BCS","BEQ","BIT","BMI",
						"BNE","BPL","BRK","BVC","BVS","CLC","CLD","CLI",
						"CLV","CMP","CPX","CPY","DEC","DEX","DEY","EOR",
						"INC","INX","INY","JMP","JSR","LDA","LDX","LDY",
						"LSR","NOP","ORA","PHA","PHP","PLA","PLP","ROL",
						"ROR","RTI","RTS","SBC","SEC","SED","SEI","STA",
						"STX","STY","TAX","TAY","TSX","TXA","TXS","TYA"};

void assemble_6502(hh_darray_t *tokens, FILE *outf){
	token_t token;
	uint32_t head = 0;
	uint32_t address = 0;
	const uint32_t tokens_end = hh_darray_get_item_fill(tokens);
	while(head < tokens_end){
		hh_darray_get(tokens, head++, &token);
		if(is_instruction(&token) != 255){
			fprintf(outf, "%s\n", token.text);
		}
		else{
			parse(tokens, &head, &address);
		}
		//printf("id: %d, line: %d, col: %d, text: '%s'\n", 
		//		token.id, token.line, token.col, token.text);
	}
	
}

// return 255 if false, index of it if true
uint8_t is_instruction(token_t *token){
	if(token->id != WORD) return 255;
	if(strlen(token->text) != 3) return 255;
	for(uint8_t i = 0; i < 56; i++){
		uint8_t j;
		for(j = 0;  j < 3; j++){
			if(char_upper(token->text[j]) != inst_words[i][j]){
				j = 254;
			}
		}
		if(j == 3) return i;
	}
	return 255;
}

char char_upper(char c){
	if(97 <= c && c <= 122){
		return c - 32;
	}
	return c;
}
