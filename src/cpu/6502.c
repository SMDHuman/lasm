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

static char char_upper(char c);
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
  if(inst_id == 255){
    // Handle unknown instruction
    print_error_loc(token);
    printf("Unknown instruction: %s\n", token->text);
    return ERR;
  }
  hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume instruction
  // Determine addressing mode
  addressing_modes_e addr_mode = 0;
  if(token->id == HASH){
    hh_darray_pop(lasm_assembler.tokens, 0, 0); // consume '#'
    addr_mode = ADM_IMM;
  }
  else if(token->id == NEWLINE){
    addr_mode = ADM_IMPL; 
  }
  else if(token->id == WORD && 
          strlen(token->text) == 1 && 
          char_upper(token->text[0]) == 'A'){
        addr_mode = ADM_ACCUM;
  }
  else{
    if(inst_addrs_mods[inst_id] == ADM_REL){
      addr_mode = ADM_REL;
    }else{
      addr_mode = ADM_ZPG|ADM_ABS;
    }
  }
 // Handle immediate values
  hh_darray_t value_bytes; hh_darray_init(&value_bytes, 1);
  if(addr_mode & (ADM_ZPG|ADM_ABS)){
    if(lasm_parse_expression(lasm_assembler.tokens, &value_bytes, 0, -1) == ERR) return ERR;
    if(hh_darray_get_fill(&value_bytes) == 1){
      addr_mode = ADM_ZPG;
    }else if (hh_darray_get_fill(&value_bytes) == 2){
      addr_mode = ADM_ABS;
    }else{
      print_error_loc(token);
      printf("[ERROR] Too big number for this addressing mode.\n");
      return ERR;
    }
  }
  else if(addr_mode == ADM_IMM){
    if(lasm_parse_expression(lasm_assembler.tokens, &value_bytes, 0, -1) == ERR) return ERR;
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
  //=================================================
  // Is this addressing mode valid with instruction
  if(!(inst_addrs_mods[inst_id] & addr_mode)){
    print_error_loc(token);
    printf("[ERROR] Addressing mode not valid for instruction '%s'\n", token->text);
    return ERR;
  }
  // Get instruction byte
  uint8_t instruction_byte = get_value_of_instruction(inst_id, addr_mode);
 
  fputc(instruction_byte, lasm_assembler.output_file);
  for(uint8_t i = 0; i < hh_darray_get_fill(&value_bytes); i++){
    uint8_t value; hh_darray_get(&value_bytes, i, &value);
    fputc(value, lasm_assembler.output_file);
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

char char_upper(char c){
  if(97 <= c && c <= 122){
    return c - 32;
  }
  return c;
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
    case 3: // BCC
      switch (addr_mode){
        case ADM_REL: return 0x90;
      }
      break;
    case 13: // CLC
      switch (addr_mode){
        case ADM_IMPL: return 0x18;
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
  }
  return 0xFF;
}
