//-----------------------------------------------------------------------------
// lasm_parser.h 14.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM_PARSER_H
#define LASM_PARSER_H

#include "lasm_tokenizer.h"
#include "hh_darray.h"

typedef struct {
  token_t token;
  struct expression_tree_t *left;
  struct expression_tree_t *right;
} expression_tree_t;

typedef struct{
  uint8_t size; // Size of the expression in bytes
  uint32_t offset; // Offset in the output file
  expression_tree_t *root; // Tokens that make up the expression
  hh_darray_t expression_tree_buffer; // Buffer for expression tree nodes
} expression_t;

uint8_t parser_expression(hh_darray_t *tokens, expression_t *expr);
uint8_t parser_expression_right(hh_darray_t *tokens, int32_t precedence, hh_darray_t *expression_tree_buffer, expression_tree_t **expr_tree_out);
void print_expression_tree(expression_tree_t *root);

#endif // LASM_PARSER_H