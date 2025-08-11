//-----------------------------------------------------------------------------
// lasm_assembler.h 10.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM_ASSEMBLER_H
#define LASM_ASSEMBLER_H

#include "lasm_namespace.h"
#include "lasm_tokenizer.h"

// Function to assemble the code
uint8_t lasm_assemble(hh_darray_t *tokens, FILE *output);

#endif // LASM_ASSEMBLER_H