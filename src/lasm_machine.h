#ifndef LASM_MACHINE_H
#define LASM_MACHINE_H

#include <stdint.h>
#include "lasm_tokenizer.h"
#include "lasm_assembler.h"

typedef struct{
  const char *mnemonic;
  uint8_t opcode;
  uint8_t bytes;
} lasm_mcode_recipe_t;

uint8_t lasm_machine_assemble_instruction(hh_darray_t* tokens, lasm_mcode_recipe_t* recipes, size_t recipe_count, const char* tag);

#endif /* LASM_MACHINE_H */