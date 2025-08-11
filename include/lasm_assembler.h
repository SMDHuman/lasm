//-----------------------------------------------------------------------------
// lasm_assembler.h 10.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM_ASSEMBLER_H
#define LASM_ASSEMBLER_H

#include "lasm_namespace.h"
#include "lasm_tokenizer.h"
#include "hh_darray.h"

typedef struct{
  uint8_t addressing_size; // Size of label in bytes
} assembler_config_t;

typedef struct{
  uint8_t size; // Size of the patch in bytes
  uint32_t offset; // Offset in the output file
  hh_darray_t tokens; // Expression to parse for patching
  namespace_t *namespace; // Namespace to use while patching
}backward_patch_t;

extern assembler_config_t lasm_config;

// Function to assemble the code
uint8_t lasm_assemble(hh_darray_t *tokens, FILE *output);
uint8_t lasm_parse_expression(hh_darray_t *tokens, FILE *output);
uint8_t lasm_token_to_number(token_t *token, uint32_t *number);
uint8_t lasm_put_number_to_file(uint32_t number, FILE *output);
uint8_t lasm_eval_token(token_t *token, uint32_t *out_number);

#endif // LASM_ASSEMBLER_H