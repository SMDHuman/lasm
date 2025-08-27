//-----------------------------------------------------------------------------
// lasm_macro.h
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM_MACRO_H
#define LASM_MACRO_H

#include <stdint.h>
#include "hh_darray.h"
#include "lasm_tokenizer.h"

//-----------------------------------------------------------------------------
uint8_t lasm_find_apply_includes(hh_darray_t *tokens, hh_darray_t *include_paths);
uint8_t lasm_extract_macros(hh_darray_t *tokens, hh_darray_t *macros);
uint8_t lasm_apply_macros(hh_darray_t *tokens, hh_darray_t *macros);
uint8_t lasm_clear_multi_newlines(hh_darray_t *tokens);
uint8_t lasm_newline_after_branches(hh_darray_t *tokens);
hh_darray_t * lasm_find_and_get_macro(token_t *token, hh_darray_t *macros);

#endif
