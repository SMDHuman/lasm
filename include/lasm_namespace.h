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
  uint8_t is_vector; // Whether the label changes the address pointer
  uint8_t is_valid; // Whether the label is evaluated
  uint32_t value; 
  token_t name;
  hh_darray_t vector_expression; // list of tokens
}label_t;

extern namespace_t global_space; // Global namespace for labels
extern hh_darray_t all_namespaces;

uint8_t lasm_namespace_init(hh_darray_t *tokens);
void lasm_namespace_to_json(namespace_t *namespace, FILE *file, int indent_level);
label_t* lasm_find_label_in_namespace(namespace_t *namespace, const char *name);
namespace_t* lasm_find_namespace_from_all(const char *name);

#endif // LASM_NAMESPACE_H