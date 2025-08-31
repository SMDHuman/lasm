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
uint8_t lasm_namespace_deinit(namespace_t *namespace);

typedef struct{
  token_t name;
  expression_t expression;
  uint64_t value; // Value of the label 
  uint8_t is_vector; // Whether the label changes the address pointer
  uint8_t is_evaluated; // Whether the label value has been evaluated
}label_t;
uint8_t lasm_label_deinit(label_t *label);

typedef struct{
  uint8_t addressing_size; // Size of label in bytes
  size_t last_address_set;
  size_t current_address_limit;
  hh_darray_t backward_patches; // Patches to apply after first pass
  namespace_t *current_namespace;
  hh_darray_t *tokens; // Currently processing tokens
  FILE* output_file; // Output file for assembled code
  uint8_t (*machine_assemble)(void); // Function to assemble machine code
  namespace_t global_namespace; // Most upper namespace
} assembler_t;

extern assembler_t lasm_assembler;

// Function to assemble the code
uint8_t lasm_assemble(hh_darray_t *tokens, FILE *output);
uint8_t lasm_eval_and_backward_patch_expression(uint8_t enable_skip);
uint8_t lasm_parse_and_eval_expression(hh_darray_t* tokens, hh_bigint_t* result, uint8_t enable_backward_patch, uint8_t is_relative, size_t max_size);
size_t lasm_scout_namespace(hh_darray_t* tokens, size_t start_from, namespace_t *namespace);
size_t lasm_get_file_size();
size_t lasm_get_file_cursor();
uint8_t lasm_token_to_number(token_t *token, hh_bigint_t *number);
uint8_t lasm_put_bytes_to_file(hh_darray_t* bytes, FILE *output);
uint8_t lasm_put_number_to_file(uint32_t number, FILE *output);
uint8_t lasm_expect_id(hh_darray_t *tokens, TOKEN_ID expected);
uint8_t lasm_expect_and_skip_id(hh_darray_t *tokens, TOKEN_ID expected);
uint8_t lasm_is_lineend_id(hh_darray_t *tokens, uint32_t index, TOKEN_ID id);
uint8_t lasm_is_lineend_text(hh_darray_t *tokens, uint32_t index, const char* text);
uint8_t lasm_evaluate_expression_tree(expression_tree_t *node, hh_bigint_t *number);
void lasm_export_json_namespace(namespace_t* ns, FILE* file, uint8_t indent_level);
label_t* lasm_find_label_in_namespace(namespace_t* namespace, const char* name);
label_t* lasm_find_label_reachable_namespace(namespace_t* namespace, const char* name);
namespace_t* lasm_find_namespace_reachable_namespace(namespace_t* namespace, const char* name);

#endif // LASM_ASSEMBLER_H