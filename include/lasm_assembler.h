//-----------------------------------------------------------------------------
// lasm_assembler.h 14.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM_ASSEMBLER_H
#define LASM_ASSEMBLER_H

#include "lasm_namespace.h"
#include "lasm_tokenizer.h"
#include "hh_darray.h"

#define DEFAULT_ADDRESSING_SIZE 2

typedef struct{
  uint8_t addressing_size; // Size of label in bytes
  hh_darray_t backward_patches; // Patches to apply after first pass
  namespace_t *current_namespace;
  uint32_t unnamed_namespace_index;
  hh_darray_t *tokens; // Currently processing tokens
  FILE* output_file; // Output file for assembled code
  uint8_t (*machine_assemble)(void); // Function to assemble machine code
} assembler_t;


typedef struct{
  uint8_t size; // Size of the patch in bytes
  uint32_t offset; // Offset in the output file
  TOKEN_ID operation; // Special operation while applying the patch. +, - etc.
  label_t *label; // Label to patch
}backward_patch_t;

extern assembler_t lasm_assembler;

// Function to assemble the code
uint8_t lasm_assemble(hh_darray_t *tokens, FILE *output);
uint8_t lasm_parse_expression(hh_darray_t *tokens, hh_darray_t *out_bytes, uint8_t stash_bwp, uint32_t max_size);
uint8_t lasm_token_to_number(token_t *token, uint32_t *number);
uint8_t lasm_put_bytes_to_file(hh_darray_t* bytes, FILE *output);
uint8_t lasm_put_number_to_file(uint32_t number, FILE *output);
uint8_t lasm_eval_token(token_t *token, uint32_t *out_number, uint32_t *size, TOKEN_ID operation, uint8_t stash_bwp, uint32_t max_size);
uint8_t lasm_check_indexing(hh_darray_t* tokens, uint32_t* number);

#endif // LASM_ASSEMBLER_H