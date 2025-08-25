//-----------------------------------------------------------------------------
// lasm_parser.h is part of the LASM (LOTP Assembler) project.
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM_PARSER_H
#define LASM_PARSER_H

#include "lasm_tokenizer.h"
#include "hh_darray.h"

typedef struct {
  token_t token;
  void *left; // expression_tree_t
  void *right; // expression_tree_t
} expression_tree_t;

typedef struct{
  uint8_t size; // Size of the expression in bytes
  uint32_t offset; // Offset in the output file
  expression_tree_t *root; // Tokens that make up the expression
  int32_t last_low_precedence;
  hh_darray_t expression_tree_buffer; // Buffer for expression tree nodes
} expression_t;

uint8_t parser_expression(hh_darray_t *tokens, expression_t *expr);
expression_tree_t* parser_expression_right(hh_darray_t *tokens, expression_t *expr, int32_t precedence);
void print_expression_tree(expression_tree_t *root);

#endif // LASM_PARSER_H