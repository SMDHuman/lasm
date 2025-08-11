#include <stdint.h>
#include <string.h>
#include "lasm_tokenizer.h"
#include "hh_darray.h"
#include "lasm_assembler.h"

char char_upper(char c);
uint8_t is_instruction(token_t *token);

char inst_words[56][3]={"ADC","AND","ASL","BCC","BCS","BEQ","BIT","BMI",
						"BNE","BPL","BRK","BVC","BVS","CLC","CLD","CLI",
						"CLV","CMP","CPX","CPY","DEC","DEX","DEY","EOR",
						"INC","INX","INY","JMP","JSR","LDA","LDX","LDY",
						"LSR","NOP","ORA","PHA","PHP","PLA","PLP","ROL",
						"ROR","RTI","RTS","SBC","SEC","SED","SEI","STA",
						"STX","STY","TAX","TAY","TSX","TXA","TXS","TYA"};

void lasm_6502_init(){
	lasm_config.addressing_size = 2; // 16-bit addressing
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
