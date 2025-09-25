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
  <nop 0x00 >; <NOP 0x00 >\
  <pha 0x10 | >; <PHA 0x10 | >\
  <sab 0x20 >; <SAB 0x20 >\
  <sab_if 0x20 | >; <SAB_IF 0x20 | >\
  <sax 0x30 >; <SAX 0x30 >\
  <sax_if 0x30 | >; <SAX_IF 0x30 | >\
  <sbx 0x40 >; <SBX 0x40 >\
  <sbx_if 0x40 | >; <SBX_IF 0x40 | >\
  <lda 0x50 >; <LDA 0x50 >\
  <lda_if 0x50 | >; <LDA_IF 0x50 | >\
  <sta 0x60 >; <STA 0x60 >\
  <sta_if 0x60 | >; <STA_IF 0x60 | >\
  <add 0x70 >; <ADD 0x70 >\
  <add_if 0x70 | >; <ADD_IF 0x70 | >\
  <sub 0x80 >; <SUB 0x80 >\
  <sub_if 0x80 | >; <SUB_IF 0x80 | >\
  <and 0x90 >; <AND 0x90 >\
  <and_if 0x90 | >; <AND_IF 0x90 | >\
  <ora 0xA0 >; <ORA 0xA0 >\
  <ora_if 0xA0 | >; <ORA_IF 0xA0 | >\
  <inc 0xB0 >; <INC 0xB0 >\
  <inc_if 0xB0 | >; <INC_IF 0xB0 | >\
  <dec 0xC0 >; <DEC 0xC0 >\
  <dec_if 0xC0 | >; <DEC_IF 0xC0 | >\
  <jmp 0xD0 >; <JMP 0xD0 >\
  <jmp_if 0xD0 | >; <JMP_IF 0xD0 | >\
  <out 0xE0 | >; <OUT 0xE0 | >\
  <inp 0xF0 | >; <INP 0xF0 | >\
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