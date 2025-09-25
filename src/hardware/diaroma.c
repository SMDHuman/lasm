#include <stdint.h>
#include <string.h>
#include "lasm_tokenizer.h"
#include "hh_darray.h"
#include "lasm_assembler.h"
#include "lasm_macro.h"

/*
RAM: 8 bit address bus
ROM: 16 bit address bus

instruction byte: hhhh llll
    hhhh: OPCODE
    llll: OPERAND

Registers: 
    A: byte
    B: byte
    X: byte
    PC: word
    Out: byte[16]
    In: byte[16]

Conditions: geci
    g: A greater than B 
    e: A and B equal
    c: Carry flag set
    i: Invert all conditions

0x0 - nop        : Do nothing
0x1 - pha 0bxxxx : Push 4 bit to A
0x2 - sab {cond} : Sawp A and B
0x3 - sax {cond} : Sawp A and X
0x4 - sbx {cond} : Sawp B and X
0x5 - lda {cond} : Load to A from RAM using X as address
0x6 - sta {cond} : Store to A from RAM using X as address
0x7 - add {cond} : Add B to A
0x8 - and {cond} : And operation B to A
0x9 - ora {cond} : Or operation B to A
0xA - sub {cond} : Subtract B from A
0xB - inc {cond} : Increment A
0xC - dec {cond} : Decrement A
0xD - jmp {cond} : Push A and X to PC : A + (X<<8) => PC
0xE - out 0bxxxx : Put X to selected Out port
0xF - inp 0bxxxx : Put selected Out port to X

*/

static const char inst_macros[] = "\
  <__pha__ 0x00>\
  <__tab__ 0x10>\
  <__sax__ 0x20>\
  <__lda__ 0x30>\
  <__sta__ 0x40>\
  <__jmp__ 0x50>\
  <__out__ 0x60>\
  <__inp__ 0x70>\
  <__add__ 0x80>\
  <__sub__ 0x90>\
  <__and__ 0xA0>\
  <__ora__ 0xB0>\
  <__xor__ 0xC0>\
  <__inc__ 0xD0>\
  <__dec__ 0xE0>\
  <__szr__ 0xF0>\
  <pha __pha__ | >; <PHA __pha__ | >\
  <tab __tab__ >; <TAB __tab__ >\
  <tab_if __tab__ | >; <TAB_IF __tab__ | >\
  <sax __sax__ >; <SAX __sax__ >\
  <sax_if __sax__ | >; <SAX_IF __sax__ | >\
  <lda __lda__ >; <LDA __lda__ >\
  <lda_if __lda__ | >; <LDA_IF __lda__ | >\
  <sta __sta__ >; <STA __sta__ >\
  <sta_if __sta__ | >; <STA_IF __sta__ | >\
  <add __add__ >; <ADD __add__ >\
  <add_if __add__ | >; <ADD_IF __add__ | >\
  <sub __sub__ >; <SUB __sub__ >\
  <sub_if __sub__ | >; <SUB_IF __sub__ | >\
  <and __and__ >; <AND __and__ >\
  <and_if __and__ | >; <AND_IF __and__ | >\
  <ora __ora__ >; <ORA __ora__ >\
  <ora_if __ora__ | >; <ORA_IF __ora__ | >\
  <xor __xor__ >; <XOR __xor__ >\
  <xor_if __xor__ | >; <XOR_IF __xor__ | >\
  <inc __inc__ >; <INC __inc__ >\
  <inc_if __inc__ | >; <INC_IF __inc__ | >\
  <dec __dec__ >; <DEC __dec__ >\
  <dec_if __dec__ | >; <DEC_IF __dec__ | >\
  <szr __szr__ >; <SZR __szr__ >\
  <szr_if __szr__ | >; <SZR_IF __szr__ | >\
  <jmp __jmp__ >; <JMP __jmp__ >\
  <jmp_if __jmp__ | >; <JMP_IF __jmp__ | >\
  <out __out__ | >; <OUT __out__ | >\
  <inp __inp__ | >; <INP __inp__ | >\
  <greater 0b0001 >; <GREATER 0b0001 >\
  <ngreater 0b1001 >; <NGREATER 0b1001 >\
  <equal 0b0010 >; <EQUAL 0b0010 >\
  <nequal 0b1010 >; <NEQUAL 0b1010 >\
  <carry 0b0100 >; <CARRY 0b0100 >\
  <ncarry 0b1100 >; <NCARRY 0b1100 >\
  <q0 0b0000 >; <Q0 0b0000 >\
  <q1 0b0001 >; <Q1 0b0001 >\
  <q2 0b0010 >; <Q2 0b0010 >\
  <q3 0b0011 >; <Q3 0b0011 >\
  <q4 0b0100 >; <Q4 0b0100 >\
  <q5 0b0101 >; <Q5 0b0101 >\
  <q6 0b0110 >; <Q6 0b0110 >\
  <q7 0b0111 >; <Q7 0b0111 >\
  <q8 0b1000 >; <Q8 0b1000 >\
  <q9 0b1001 >; <Q9 0b1001 >\
  <q10 0b1010 >; <Q10 0b1010 >\
  <q11 0b1011 >; <Q11 0b1011 >\
  <q12 0b1100 >; <Q12 0b1100 >\
  <q13 0b1101 >; <Q13 0b1101 >\
  <q14 0b1110 >; <Q14 0b1110 >\
  <q15 0b1111 >; <Q15 0b1111 >\
  <p0 0b0000 >; <P0 0b0000 >\
  <p1 0b0001 >; <P1 0b0001 >\
  <p2 0b0010 >; <P2 0b0010 >\
  <p3 0b0011 >; <P3 0b0011 >\
  <p4 0b0100 >; <P4 0b0100 >\
  <p5 0b0101 >; <P5 0b0101 >\
  <p6 0b0110 >; <P6 0b0110 >\
  <p7 0b0111 >; <P7 0b0111 >\
  <p8 0b1000 >; <P8 0b1000 >\
  <p9 0b1001 >; <P9 0b1001 >\
  <p10 0b1010 >; <P10 0b1010 >\
  <p11 0b1011 >; <P11 0b1011 >\
  <p12 0b1100 >; <P12 0b1100 >\
  <p13 0b1101 >; <P13 0b1101 >\
  <p14 0b1110 >; <P14 0b1110 >\
  <p15 0b1111 >; <P15 0b1111 >\
";

uint8_t lasm_diaroma_init(assembler_t *assembler){
  assembler->addressing_size = 1; // ?
  assembler->address_size_out_of_range_warning = 0;
  hh_darray_t tokens; hh_darray_init(&tokens, sizeof(token_t));
  FILE *input_file = fopen("diaroma.l", "w+");
  fwrite(inst_macros, 1, sizeof(inst_macros)-1, input_file);
  fseek(input_file, 0, SEEK_SET);
  if(lasm_tokenize(input_file, "hardware/diaroma.l\0", &tokens) == ERR) return ERR;
  fclose(input_file);
  remove("diaroma.l");
  //====================================
  hh_darray_t macros; hh_darray_init(&macros, sizeof(macro_t));
  if(lasm_extract_macros(&tokens, &macros) == ERR) return ERR;
  //print_macros(&macros);
  // Apply macros main tokens
  if(lasm_apply_macros(assembler->tokens, &macros) == ERR) return ERR;
  if(lasm_newline_after_branches(assembler->tokens) == ERR) return ERR;
  if(lasm_clear_multi_newlines(assembler->tokens) == ERR) return ERR;

  return 0;
}

uint8_t lasm_diaroma_assemble(assembler_t *assembler){
  return *(uint8_t*)assembler; // unused value
}