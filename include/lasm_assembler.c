//-----------------------------------------------------------------------------
// lasm_assembler.c 10.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "lasm_assembler.h"
#include "lasm_tokenizer.h"

// Function to assemble the code
uint8_t lasm_assemble(hh_darray_t *tokens, FILE *output){
  token_t *token;
  namespace_t *current_namespace = &global_space;
  while(1){
    token = hh_darray_get_reference(tokens, 0);
    if(token->id == WORD){
      // Handle word token
    }
    else if(token->id == NUMBER){
      // Handle number token
    }
    else if(token->id == RBRAC_O){
      // Handle '(' token
    }
    else{
      print_error_loc(token);
      printf("Unexpected token: %s\n", token->text);
      return ERR;
    }
  }
  return 0;
}
