#include <stdint.h>
#include <string.h>
#include "lasm_tokenizer.h"
#include "hh_darray.h"
#include "lasm_assembler.h"

/* Address Modes
  A				Accumulator						OPC A					operand is AC (implied single byte instruction)
  abs			absolute							OPC $LLHH			operand is address $HHLL *
  abs,X		absolute, X-indexed		OPC $LLHH,X		operand is address; effective address is address incremented by X with carry **
  abs,Y		absolute, Y-indexed		OPC $LLHH,Y		operand is address; effective address is address incremented by Y with carry **
  #				immediate							OPC #$BB			operand is byte BB
  impl		implied								OPC						operand implied
  ind			indirect							OPC ($LLHH)		operand is address; effective address is contents of word at address: C.w($HHLL)
  X,ind		X-indexed, indirect		OPC ($LL,X)		operand is zeropage address; effective address is word in (LL + X, LL + X + 1), inc. without carry: C.w($00LL + X)
  ind,Y		indirect, Y-indexed		OPC ($LL),Y		operand is zeropage address; effective address is word in (LL, LL + 1) incremented by Y with carry: C.w($00LL) + Y
  rel			relative							OPC $BB				branch target is PC + signed offset BB ***
  zpg			zeropage							OPC $LL				operand is zeropage address (hi-byte is zero, address = $00LL)
  zpg,X		zeropage, X-indexed		OPC $LL,X			operand is zeropage address; effective address is address incremented by X without carry **
  zpg,Y		zeropage, Y-indexed		OPC $LL,Y			operand is zeropage address; effective address is address incremented by Y without carry **
*/

char addressing_mode_words[16][16] = {
  "accumulator",
  "absolute",
  "absolute,X",
  "absolute,Y",
  "immediate",
  "implied",
  "indirect",
  "X,indirect",
  "indirect,Y",
  "relative",
  "zeropage",
  "zeropage,X",
  "zeropage,Y"
};

typedef enum{
  ADM_ACCUM = 1<<0,
  ADM_ABS = 1<<1,
  ADM_ABS_X = 1<<2,
  ADM_ABS_Y = 1<<3,
  ADM_IMM = 1<<4,
  ADM_IMPL = 1<<5,
  ADM_IND = 1<<6,
  ADM_X_IND = 1<<7,
  ADM_IND_Y = 1<<8,
  ADM_REL = 1<<9,
  ADM_ZPG = 1<<10,
  ADM_ZPG_X = 1<<11,
  ADM_ZPG_Y = 1<<12,
}addressing_modes_e;

uint32_t addressing_modes_index_map(addressing_modes_e mode) {
  switch (mode) {
    case ADM_ACCUM: return 0;
    case ADM_ABS: return 1;
    case ADM_ABS_X: return 2;
    case ADM_ABS_Y: return 3;
    case ADM_IMM: return 4;
    case ADM_IMPL: return 5;
    case ADM_IND: return 6;
    case ADM_X_IND: return 7;
    case ADM_IND_Y: return 8;
    case ADM_REL: return 9;
    case ADM_ZPG: return 10;
    case ADM_ZPG_X: return 11;
    case ADM_ZPG_Y: return 12;
    default: return mode; // Return the mode as index if not found
  }
}

static uint8_t is_instruction(token_t *token);

static uint8_t get_value_of_instruction(uint8_t inst_id, addressing_modes_e addr_mode);

static char inst_words[56][3]={"ADC","AND","ASL","BCC","BCS","BEQ","BIT","BMI",
                                "BNE","BPL","BRK","BVC","BVS","CLC","CLD","CLI",
                                "CLV","CMP","CPX","CPY","DEC","DEX","DEY","EOR",
                                "INC","INX","INY","JMP","JSR","LDA","LDX","LDY",
                                "LSR","NOP","ORA","PHA","PHP","PLA","PLP","ROL",
                                "ROR","RTI","RTS","SBC","SEC","SED","SEI","STA",
                                "STX","STY","TAX","TAY","TSX","TXA","TXS","TYA"};


static addressing_modes_e inst_addrs_mods[56]={
  /*ADC*/ ADM_IMM|ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X|ADM_ABS_Y|ADM_X_IND|ADM_IND_Y,
  /*AND*/ ADM_IMM|ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X|ADM_ABS_Y|ADM_X_IND|ADM_IND_Y,
  /*ASL*/ ADM_ACCUM|ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X,
  /*BCC*/ ADM_REL,
  /*BCS*/ ADM_REL,
  /*BEQ*/ ADM_REL,
  /*BIT*/ ADM_ZPG|ADM_ABS,
  /*BMI*/ ADM_REL,
  /*BNE*/ ADM_REL,
  /*BPL*/ ADM_REL,
  /*BRK*/ ADM_IMPL,
  /*BVC*/ ADM_REL,
  /*BVS*/ ADM_REL,
  /*CLC*/ ADM_IMPL,
  /*CLD*/ ADM_IMPL,
  /*CLI*/ ADM_IMPL,
  /*CLV*/ ADM_IMPL,
  /*CMP*/ ADM_IMM|ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X|ADM_ABS_Y|ADM_X_IND|ADM_IND_Y,
  /*CPX*/ ADM_IMM|ADM_ZPG|ADM_ABS,
  /*CPY*/ ADM_IMM|ADM_ZPG|ADM_ABS,
  /*DEC*/ ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X,
  /*DEX*/ ADM_IMPL,
  /*DEY*/ ADM_IMPL,
  /*EOR*/ ADM_IMM|ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X|ADM_ABS_Y|ADM_X_IND|ADM_IND_Y,
  /*INC*/ ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X,
  /*INX*/ ADM_IMPL,
  /*INY*/ ADM_IMPL,
  /*JMP*/ ADM_ABS|ADM_IND,
  /*JSR*/ ADM_ABS,
  /*LDA*/ ADM_IMM|ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X|ADM_ABS_Y|ADM_X_IND|ADM_IND_Y,
  /*LDX*/ ADM_IMM|ADM_ZPG|ADM_ZPG_Y|ADM_ABS|ADM_ABS_Y,
  /*LDY*/ ADM_IMM|ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X,
  /*LSR*/ ADM_ACCUM|ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X,
  /*NOP*/ ADM_IMPL,
  /*ORA*/ ADM_IMM|ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X|ADM_ABS_Y|ADM_X_IND|ADM_IND_Y,
  /*PHA*/ ADM_IMPL,
  /*PHP*/ ADM_IMPL,
  /*PLA*/ ADM_IMPL,
  /*PLP*/ ADM_IMPL,
  /*ROL*/ ADM_ACCUM|ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X,
  /*ROR*/ ADM_ACCUM|ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X,
  /*RTI*/ ADM_IMPL,
  /*RTS*/ ADM_IMPL,
  /*SBC*/ ADM_IMM|ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X|ADM_ABS_Y|ADM_X_IND|ADM_IND_Y,
  /*SEC*/ ADM_IMPL,
  /*SED*/ ADM_IMPL,
  /*SEI*/ ADM_IMPL,
  /*STA*/ ADM_ZPG|ADM_ZPG_X|ADM_ABS|ADM_ABS_X|ADM_ABS_Y|ADM_X_IND|ADM_IND_Y,
  /*STX*/ ADM_ZPG|ADM_ZPG_Y|ADM_ABS,
  /*STY*/ ADM_ZPG|ADM_ZPG_X|ADM_ABS,
  /*TAX*/ ADM_IMPL,
  /*TAY*/ ADM_IMPL,
  /*TSX*/ ADM_IMPL,
  /*TXA*/ ADM_IMPL,
  /*TXS*/ ADM_IMPL,
  /*TYA*/ ADM_IMPL
};

void lasm_6502_init(){
  lasm_assembler.addressing_size = 2; // 16-bit addressing
}

uint8_t lasm_6502_assemble(void){
  token_t *token = hh_darray_get_reference(lasm_assembler.tokens, 0);
  uint8_t inst_id = is_instruction(token);
  printf("instruction token: %s\n", token->text);
  if(inst_id == 255){
    // Handle unknown instruction
    print_error_loc(token);
    printf("Unknown instruction: %s\n", token->text);
    return ERR;
  }
  hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume instruction
  //==================================
  // Determine addressing mode
  addressing_modes_e addr_mode = 0;
  // Immediate mode
  if(token->id == HASH){
    hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume '#'
    addr_mode = ADM_IMM;
  }
  // Implied mode
  else if(token->id == NEWLINE){
    addr_mode = ADM_IMPL; 
  }
  // Accumulator mode
  else if(token->id == WORD && 
          strlen(token->text) == 1 && 
          char_upper(token->text[0]) == 'A'){
        addr_mode = ADM_ACCUM;
    hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume 'A'
  }
  // Indirect mode
  else if(token->id == RBRAC_O && 
          (is_lineend_token_id(lasm_assembler.tokens, 0, RBRAC_C) || 
          is_lineend_token_text(lasm_assembler.tokens, 0, "Y"))){
    addr_mode = ADM_IND;
    hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume '('
  }
  // Relative, Zero Page or Absolute mode
  else{
    // Prioritize relative mode if instruction supports it
    if(inst_addrs_mods[inst_id] & ADM_REL){
      addr_mode = ADM_REL;
    }else{
      addr_mode = ADM_ZPG|ADM_ABS;
    }
  }
  //==================================
  // Handle argument values
  hh_darray_t value_bytes; hh_darray_init(&value_bytes, 1);
  if(addr_mode & (ADM_ZPG|ADM_ABS)){
    if(lasm_parse_expression(lasm_assembler.tokens, &value_bytes, 1, -1) == ERR) return ERR;
    if(hh_darray_get_fill(&value_bytes) == 1){
      if(token->id == COMMA){
        hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume ','
        if(char_upper(token->text[0]) == 'X' && strlen(token->text) == 1){
          hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume 'X'
          addr_mode = ADM_ZPG_X;
        }else if(char_upper(token->text[0]) == 'Y' && strlen(token->text) == 1){
          hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume 'Y'
          addr_mode = ADM_ZPG_Y;
        }else{
          print_error_loc(token);
          printf("[ERROR] Expected 'X' or 'Y' after ','\n");
          return ERR;
        }
      }else{
        addr_mode = ADM_ZPG;        
      }
    }else if (hh_darray_get_fill(&value_bytes) == 2){
      if(token->id == COMMA){
        hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume ','
        if(char_upper(token->text[0]) == 'X' && strlen(token->text) == 1){
          hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume 'X'
          addr_mode = ADM_ABS_X;
        }else if(char_upper(token->text[0]) == 'Y' && strlen(token->text) == 1){
          hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume 'Y'
          addr_mode = ADM_ABS_Y;
        }else{
          print_error_loc(token);
          printf("[ERROR] Expected 'X' or 'Y' after ','\n");
          return ERR;
        }
      }else{
        addr_mode = ADM_ABS;        
      }
    }
  }
  else if(addr_mode == ADM_IMM){
    if(lasm_parse_expression(lasm_assembler.tokens, &value_bytes, 1, -1) == ERR) return ERR;
    // Ensure only the least significant byte is used
    while(hh_darray_get_fill(&value_bytes) > 1){
      hh_darray_popend(&value_bytes, 0);
    }
  }
  else if(addr_mode == ADM_REL){
    token_t val_token, op_token;
    hh_darray_get(lasm_assembler.tokens, 0, &val_token);
    hh_darray_get(lasm_assembler.tokens, 0, &op_token);
    val_token.id = NUMBER;
    sprintf(val_token.text, "%ld", ftell(lasm_assembler.output_file)+2);
    op_token.id = MINUS;
    sprintf(op_token.text, "%c", '-');
    hh_darray_push(lasm_assembler.tokens, 1, &val_token);
    hh_darray_push(lasm_assembler.tokens, 1, &op_token);
    ///...
    fseek(lasm_assembler.output_file, 1, SEEK_CUR);
    if(lasm_parse_expression(lasm_assembler.tokens, &value_bytes, 1, 1) == ERR) return ERR;
    fseek(lasm_assembler.output_file, -1, SEEK_CUR);
    // Ensure only the least significant byte is used
    while(hh_darray_get_fill(&value_bytes) > 1){
      hh_darray_popend(&value_bytes, 0);
    }
  }
  else if(addr_mode == ADM_IND){
    if(lasm_parse_expression(lasm_assembler.tokens, &value_bytes, 1, -1) == ERR) return ERR;
    if (token->id == RBRAC_C){
      hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume ')'
      if(token->id == COMMA){
        if(hh_darray_get_fill(&value_bytes) > 1){
          print_error_loc(token);
          printf("[ERROR] Expected 1 byte but got %lu bytes\n", hh_darray_get_fill(&value_bytes));
          return ERR;
        }
        hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume ','
        if(char_upper(token->text[0]) == 'Y' && strlen(token->text) == 1){
          hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume 'Y'
          addr_mode = ADM_IND_Y;
        }
        else{
          print_error_loc(token);
          printf("Expected 'Y' after ','\n");
          return ERR;
        }
      }
      else{
        if(hh_darray_get_fill(&value_bytes) < 2){
          // Ensure its 2 byte long
          hh_darray_append(&value_bytes, 0);
        }
      }
    }
    else{
      // Ensure its 1 byte from code
      if(hh_darray_get_fill(&value_bytes) > 1){
        print_error_loc(token);
        printf("[ERROR] Expected 1 byte but got %lu bytes\n", hh_darray_get_fill(&value_bytes));
        return ERR;
      }
      if(token->id == COMMA){
        hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume ','
        if(char_upper(token->text[0]) == 'X' && strlen(token->text) == 1){
          hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume 'X'
          addr_mode = ADM_X_IND;
        }else{
          print_error_loc(token);
          printf("[ERROR] Expected 'X' after ','\n");
          return ERR;
        }
        
        if(token->id != RBRAC_C){
          print_error_loc(token);
          printf("Expected a right bracket ')' but got '%s'\n", token->text);
          return ERR;
        }
        hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume ')'
      }
      else{
        print_error_loc(token);
        printf("Invalid Indirect Addressing Mode\n");
        return ERR;
      }
    }
  }
  //=================================================
  // Is this addressing mode valid with instruction
  if(!(inst_addrs_mods[inst_id] & addr_mode)){
    print_error_loc(token);
    char word[4]={0}; memcpy(&word, inst_words[inst_id], 3); 
    printf("Addressing mode '%s' not valid for instruction '%s'\n", addressing_mode_words[addressing_modes_index_map(addr_mode)], word);
    return ERR;
  }
  // Get instruction byte
  uint8_t instruction_byte = get_value_of_instruction(inst_id, addr_mode);
 
  fputc(instruction_byte, lasm_assembler.output_file);
  for(uint8_t i = 0; i < hh_darray_get_fill(&value_bytes); i++){
    uint8_t value; hh_darray_get(&value_bytes, i, &value);
    fputc(value, lasm_assembler.output_file);
    printf("instruction byte: %02X ", instruction_byte);
    printf("value after that: %02X\n", value);
  }


  hh_darray_deinit(&value_bytes);
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

uint8_t get_value_of_instruction(uint8_t inst_id, addressing_modes_e addr_mode){
  switch (inst_id){
    case 0: // ADC
      switch (addr_mode){
        case ADM_IMM: return 0x69;
        case ADM_ZPG: return 0x65;
        case ADM_ZPG_X: return 0x75;
        case ADM_ABS: return 0x6D;
        case ADM_ABS_X: return 0x7D;
        case ADM_ABS_Y: return 0x79;
        case ADM_X_IND: return 0x61;
        case ADM_IND_Y: return 0x71;
      }
      break;
    case 1: // AND
      switch (addr_mode){
        case ADM_IMM: return 0x29;
        case ADM_ZPG: return 0x25;
        case ADM_ZPG_X: return 0x35;
        case ADM_ABS: return 0x2D;
        case ADM_ABS_X: return 0x3D;
        case ADM_ABS_Y: return 0x39;
        case ADM_X_IND: return 0x21;
        case ADM_IND_Y: return 0x31;
      }
      break;
    case 2: // ASL
      switch (addr_mode){
        case ADM_ACCUM: return 0x0A;
        case ADM_ZPG: return 0x06;
        case ADM_ZPG_X: return 0x16;
        case ADM_ABS: return 0x0E;
        case ADM_ABS_X: return 0x1E;
      }
      break;
    case 3: // BCC
      switch (addr_mode){
        case ADM_REL: return 0x90;
      }
      break;
    case 4: // BCS
      switch (addr_mode){
        case ADM_REL: return 0xB0;
      }
      break;
    case 5: // BEQ
      switch (addr_mode){
        case ADM_REL: return 0xF0;
      }
      break;
    case 6: // BIT
      switch (addr_mode){
        case ADM_ZPG: return 0x24;
        case ADM_ABS: return 0x2C;
      }
      break;
    case 7: // BMI
      switch (addr_mode){
        case ADM_REL: return 0x30;
      }
      break;
    case 8: // BNE
      switch (addr_mode){
        case ADM_REL: return 0xD0;
      }
      break;
    case 9: // BPL
      switch (addr_mode){
        case ADM_REL: return 0x10;
      }
      break;
    case 10: // BRK
      switch (addr_mode){
        case ADM_IMPL: return 0x00;
      }
      break;
    case 11: // BVC
      switch (addr_mode){
        case ADM_REL: return 0x50;
      }
      break;
    case 12: // BVS
      switch (addr_mode){
        case ADM_REL: return 0x70;
      }
      break;
    case 13: // CLC
      switch (addr_mode){
        case ADM_IMPL: return 0x18;
      }
      break;
    case 14: // CLD
      switch (addr_mode){
        case ADM_IMPL: return 0xD8;
      }
      break;
    case 15: // CLI
      switch (addr_mode){
        case ADM_IMPL: return 0x58;
      }
      break;
    case 16: // CLV
      switch (addr_mode){
        case ADM_IMPL: return 0xB8;
      }
      break;
    case 17: // CMP
      switch (addr_mode){
        case ADM_IMM: return 0xC9;
        case ADM_ZPG: return 0xC5;
        case ADM_ZPG_X: return 0xD5;
        case ADM_ABS: return 0xCD;
        case ADM_ABS_X: return 0xDD;
        case ADM_ABS_Y: return 0xD9;
        case ADM_X_IND: return 0xC1;
        case ADM_IND_Y: return 0xD1;
      }
      break;
    case 18: // CPX
      switch (addr_mode){
        case ADM_IMM: return 0xE0;
        case ADM_ZPG: return 0xE4;
        case ADM_ABS: return 0xEC;
      }
      break;
    case 19: // CPY
      switch (addr_mode){
        case ADM_IMM: return 0xC0;
        case ADM_ZPG: return 0xC4;
        case ADM_ABS: return 0xCC;
      }
      break;
    case 20: // DEC
      switch (addr_mode){
        case ADM_ZPG: return 0xC6;
        case ADM_ZPG_X: return 0xD6;
        case ADM_ABS: return 0xCE;
        case ADM_ABS_X: return 0xDE;
      }
      break;
    case 21: // DEX
      switch (addr_mode){
        case ADM_IMPL: return 0xCA;
      }
      break;
    case 22: // DEY
      switch (addr_mode){
        case ADM_IMPL: return 0x88;
      }
      break;
    case 23: // EOR
      switch (addr_mode){
        case ADM_IMM: return 0x49;
        case ADM_ZPG: return 0x45;
        case ADM_ZPG_X: return 0x55;
        case ADM_ABS: return 0x4D;
        case ADM_ABS_X: return 0x5D;
        case ADM_ABS_Y: return 0x59;
        case ADM_X_IND: return 0x41;
        case ADM_IND_Y: return 0x51;
      }
      break;
    case 24: // INC
      switch (addr_mode){
        case ADM_ZPG: return 0xE6;
        case ADM_ZPG_X: return 0xF6;
        case ADM_ABS: return 0xEE;
        case ADM_ABS_X: return 0xFE;
      }
      break;
    case 25: // INX
      switch (addr_mode){
        case ADM_IMPL: return 0xE8;
      }
      break;
    case 26: // INY
      switch (addr_mode){
        case ADM_IMPL: return 0xC8;
      }
      break;
    case 27: // JMP
      switch (addr_mode){
        case ADM_ABS: return 0x4C;
        case ADM_IND: return 0x6C;
      }
      break;
    case 28: // JSR
      switch (addr_mode){
        case ADM_ABS: return 0x20;
      }
      break;
    case 29: // LDA
      switch (addr_mode){
        case ADM_IMM: return 0xA9;
        case ADM_ZPG: return 0xA5;
        case ADM_ZPG_X: return 0xB5;
        case ADM_ABS: return 0xAD;
        case ADM_ABS_X: return 0xBD;
        case ADM_ABS_Y: return 0xB9;
        case ADM_X_IND: return 0xA1;
        case ADM_IND_Y: return 0xB1;
      }
      break;
    case 30: // LDX
      switch (addr_mode){
        case ADM_IMM: return 0xA2;
        case ADM_ZPG: return 0xA6;
        case ADM_ZPG_Y: return 0xB6;
        case ADM_ABS: return 0xAE;
        case ADM_ABS_Y: return 0xBE;
      }
      break;
    case 31: // LDY
      switch (addr_mode){
        case ADM_IMM: return 0xA0;
        case ADM_ZPG: return 0xA4;
        case ADM_ZPG_X: return 0xB4;
        case ADM_ABS: return 0xAC;
        case ADM_ABS_X: return 0xBC;
      }
      break;
    case 32: // LSR
      switch (addr_mode){
        case ADM_ACCUM: return 0x4A;
        case ADM_ZPG: return 0x46;
        case ADM_ZPG_X: return 0x56;
        case ADM_ABS: return 0x4E;
        case ADM_ABS_X: return 0x5E;
      }
      break;
    case 33: // NOP
      switch (addr_mode){
        case ADM_IMPL: return 0xEA;
      }
      break;
    case 34: // ORA
      switch (addr_mode){
        case ADM_IMM: return 0x09;
        case ADM_ZPG: return 0x05;
        case ADM_ZPG_X: return 0x15;
        case ADM_ABS: return 0x0D;
        case ADM_ABS_X: return 0x1D;
        case ADM_ABS_Y: return 0x19;
        case ADM_X_IND: return 0x01;
        case ADM_IND_Y: return 0x11;
      }
      break;
    case 35: // PHA
      switch (addr_mode){
        case ADM_IMPL: return 0x48;
      }
      break;
    case 36: // PHP
      switch (addr_mode){
        case ADM_IMPL: return 0x08;
      }
      break;
    case 37: // PLA
      switch (addr_mode){
        case ADM_IMPL: return 0x68;
      }
      break;
    case 38: // PLP
      switch (addr_mode){
        case ADM_IMPL: return 0x28;
      }
      break;
    case 39: // ROL
      switch (addr_mode){
        case ADM_ACCUM: return 0x2A;
        case ADM_ZPG: return 0x26;
        case ADM_ZPG_X: return 0x36;
        case ADM_ABS: return 0x2E;
        case ADM_ABS_X: return 0x3E;
      }
      break;
    case 40: // ROR
      switch (addr_mode){
        case ADM_ACCUM: return 0x6A;
        case ADM_ZPG: return 0x66;
        case ADM_ZPG_X: return 0x76;
        case ADM_ABS: return 0x6E;
        case ADM_ABS_X: return 0x7E;
      }
      break;
    case 41: // RTI
      switch (addr_mode){
        case ADM_IMPL: return 0x40;
      }
      break;
    case 42: // RTS
      switch (addr_mode){
        case ADM_IMPL: return 0x60;
      }
      break;
    case 43: // SBC
      switch (addr_mode){
        case ADM_IMM: return 0xE9;
        case ADM_ZPG: return 0xE5;
        case ADM_ZPG_X: return 0xF5;
        case ADM_ABS: return 0xED;
        case ADM_ABS_X: return 0xFD;
        case ADM_ABS_Y: return 0xF9;
        case ADM_X_IND: return 0xE1;
        case ADM_IND_Y: return 0xF1;
      }
      break;
    case 44: // SEC
      switch (addr_mode){
        case ADM_IMPL: return 0x38;
      }
      break;
    case 45: // SED
      switch (addr_mode){
        case ADM_IMPL: return 0xF8;
      }
      break;
    case 46: // SEI
      switch (addr_mode){
        case ADM_IMPL: return 0x78;
      }
      break;
    case 47: // STA
      switch (addr_mode){
        case ADM_ZPG: return 0x85;
        case ADM_ZPG_X: return 0x95;
        case ADM_ABS: return 0x8D;
        case ADM_ABS_X: return 0x9D;
        case ADM_ABS_Y: return 0x99;
        case ADM_X_IND: return 0x81;
        case ADM_IND_Y: return 0x91;
      }
      break;
    case 48: // STX
      switch (addr_mode){
        case ADM_ZPG: return 0x86;
        case ADM_ZPG_Y: return 0x96;
        case ADM_ABS: return 0x8E;
      }
      break;
    case 49: // STY
      switch (addr_mode){
        case ADM_ZPG: return 0x84;
        case ADM_ZPG_X: return 0x94;
        case ADM_ABS: return 0x8C;
      }
      break;
    case 50: // TAX
      switch (addr_mode){
        case ADM_IMPL: return 0xAA;
      }
      break;
    case 51: // TAY
      switch (addr_mode){
        case ADM_IMPL: return 0xA8;
      }
      break;
    case 52: // TSX
      switch (addr_mode){
        case ADM_IMPL: return 0xBA;
      }
      break;
    case 53: // TXA
      switch (addr_mode){
        case ADM_IMPL: return 0x8A;
      }
      break;
    case 54: // TXS
      switch (addr_mode){
        case ADM_IMPL: return 0x9A;
      }
      break;
    case 55: // TYA
      switch (addr_mode){
        case ADM_IMPL: return 0x98;
      }
      break;
  }
  return 0xFF;
}
