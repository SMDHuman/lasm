//-----------------------------------------------------------------------------
// lasm_namespace.h 10.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM_NAMESPACE_H
#define LASM_NAMESPACE_H

#include "lasm_tokenizer.h"
#include "hh_darray.h"

typedef struct{
  token_t name;
  uint32_t level; // Level of namespace determined by curly braces
  uint8_t constant; // Whether the namespace will consist after exiting of its level 
  hh_darray_t labels; // list of label_t
  hh_darray_t childs_index; // list of namespace_t
  struct namespace_t *parent; // Parent namespace for nested namespaces
}namespace_t;

typedef struct{
  token_t name; 
  uint8_t vector; // Whether the label changes the address pointer
  hh_darray_t address_expression; // list of tokens
}label_t;

extern namespace_t global_space; // Global namespace for labels
extern hh_darray_t all_namespaces;

uint8_t lasm_namespace_init(hh_darray_t *tokens);
void lasm_namespace_to_json(namespace_t *namespace, FILE *file, int indent_level);

#endif // LASM_NAMESPACE_H