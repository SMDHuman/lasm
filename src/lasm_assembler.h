//-----------------------------------------------------------------------------
// lasm_assembler.h 14.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM_ASSEMBLER_H
#define LASM_ASSEMBLER_H

#include "lasm_tokenizer.h"
#include "lasm_parser.h"
#include "hh_darray.h"
#include "hh_bigint.h"

#define DEFAULT_ADDRESSING_SIZE 2

typedef struct{
  token_t name;
  uint32_t level; // Level of namespace determined by curly braces
  uint8_t constant; // Whether the namespace will consist after exiting of its level 
  hh_darray_t labels; // list of label_t
  hh_darray_t childs; // list of namespace_t
  void *parent; // Parent namespace for nested namespaces
}namespace_t;

typedef struct{
  token_t name;
  expression_t expression;
  hh_bigint_t value; // Value of the label 
  uint8_t is_vector; // Whether the label changes the address pointer
}label_t;

typedef struct{
  uint8_t addressing_size; // Size of label in bytes
  hh_darray_t backward_patches; // Patches to apply after first pass
  namespace_t *current_namespace;
  uint32_t unnamed_namespace_count;
  hh_darray_t *tokens; // Currently processing tokens
  FILE* output_file; // Output file for assembled code
  uint8_t (*machine_assemble)(void); // Function to assemble machine code
  namespace_t global_namespace; // Most upper namespace
} assembler_t;

extern assembler_t lasm_assembler;

// Function to assemble the code
uint8_t lasm_assemble(hh_darray_t *tokens, FILE *output);
uint8_t lasm_token_to_number(token_t *token, hh_bigint_t *number);
uint8_t lasm_put_bytes_to_file(hh_darray_t* bytes, FILE *output);
uint8_t lasm_put_number_to_file(uint32_t number, FILE *output);
uint8_t lasm_expect_and_skip(hh_darray_t *tokens, TOKEN_ID expected);
uint8_t lasm_expect(hh_darray_t *tokens, TOKEN_ID expected);
uint8_t lasm_evaluate_expression_tree(expression_tree_t *node, hh_bigint_t *number);
void lasm_export_json_namespace(namespace_t* ns, FILE* file, uint8_t indent_level);
label_t* lasm_find_label_reachable_namespace(namespace_t* namespace, const char* name);

#endif // LASM_ASSEMBLER_H