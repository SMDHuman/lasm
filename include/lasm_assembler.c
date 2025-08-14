//-----------------------------------------------------------------------------
// lasm_assembler.c 14.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "lasm_assembler.h"
#include "lasm_tokenizer.h"
#include "lasm_namespace.h"
#include "lasm_parser.h"

//-----------------------------------------------------------------------------
assembler_t lasm_assembler;

static uint32_t get_size_of_file(FILE* file);

//-----------------------------------------------------------------------------
// Function to assemble the code
uint8_t lasm_assemble(hh_darray_t *tokens, FILE *output){
  //...
  lasm_assembler.current_namespace = &global_space;
  lasm_assembler.unnamed_namespace_index = 0;
  lasm_assembler.tokens = tokens;
  lasm_assembler.output_file = output;
  hh_darray_init(&lasm_assembler.backward_patches, sizeof(expression_t));
  //...
  token_t *token = hh_darray_get_reference(tokens, 0);
  while(hh_darray_get_fill(tokens) > 0){
    if(token->id == WORD){
      // Handle word token
      token_t *next_token = hh_darray_get_reference(tokens, 1);
      label_t *label = lasm_find_label_in_namespace(lasm_assembler.current_namespace, token->text);
      if(label != NULL){
        if(is_lineend_token_id(tokens, 0, COLON)){
          if(label->is_vector){
            // Handle vector token
            hh_darray_pop(tokens, 0, 0); // Consume label token
            hh_darray_pop(tokens, 0, 0); // Consume open bracket
            //...
            //TODO: Implement the evaluation of expressions
            expression_t expression;
            if(parser_expression(tokens, &expression) == ERR) return ERR;
            print_expression_tree(expression.root);printf("\n");
            //...
            if(lasm_expect_and_skip(tokens, SBRAC_C) == ERR) return ERR;
            fseek(output, label->value, SEEK_SET);
            label->is_valid = 1;
            if(lasm_expect_and_skip(tokens, COLON) == ERR) return ERR;
          }
          else{
            // Handle unknown label
            hh_darray_pop(tokens, 0, 0); // Consume label token
            hh_darray_pop(tokens, 0, 0); // Consume colon token
            //...
            label->value = get_size_of_file(output);
            label->is_valid = 1;
          }
        }else{
          //TODO: Implement the evaluation of expressions
          expression_t expression;
          if(parser_expression(tokens, &expression) == ERR) return ERR;
          print_expression_tree(expression.root);printf("\n");
        }
      }
      else if(next_token->id == CBRAC_O){
        // Enter namespace with name
        lasm_assembler.current_namespace = lasm_find_namespace_from_all(token->text);
        if (lasm_assembler.current_namespace == NULL) {
          print_error_loc(token);
          printf("Unknown namespace: %s\n", token->text);
          return ERR;
        }
        hh_darray_pop(tokens, 0, 0); // Consume label token
        hh_darray_pop(tokens, 0, 0); // Consume curly brace open
      }else{
        if(lasm_assembler.machine_assemble() == ERR) return ERR;
      }
    }
    else if(token->id == CBRAC_O){
      // Handle '{' token
      char index_str[16];
      sprintf(index_str, "__unnamed%d__", lasm_assembler.unnamed_namespace_index++);
      lasm_assembler.current_namespace = lasm_find_namespace_from_all(index_str);
      hh_darray_pop(tokens, 0, 0); // Consume curly brace open
    }
    else if(token->id == CBRAC_C){
      // Handle '}' token
      lasm_assembler.current_namespace = (namespace_t *)lasm_assembler.current_namespace->parent;
      hh_darray_pop(tokens, 0, 0); // Consume curly brace close
    }
    else if(token->id == NUMBER){
      //TODO: Implement the evaluation of expressions
      expression_t expression;
      if(parser_expression(tokens, &expression) == ERR) return ERR;
      print_expression_tree(expression.root);printf("\n");
    }
    else if(token->id == STRING_DB){
      //TODO: Implement the evaluation of expressions
      expression_t expression;
      if(parser_expression(tokens, &expression) == ERR) return ERR;
      print_expression_tree(expression.root);printf("\n");
    }
    else if(token->id == RBRAC_O){
      //TODO: Implement the evaluation of expressions
      expression_t expression;
      if(parser_expression(tokens, &expression) == ERR) return ERR;
      print_expression_tree(expression.root);printf("\n");
    }
    else{
      print_error_loc(token);
      printf("Unexpected token: %s\n", token->text);
      return ERR;
    }
    // Expect newline
    if(token->id == NEWLINE){
      hh_darray_pop(tokens, 0, 0);
    }else {
      print_error_loc(token);
      printf("Expected newline but got '%s'\n", token->text);
      return ERR;
    }
  }
  // =====================
  // Backwards patches
  
  return 0;
}
//-----------------------------------------------------------------------------
uint8_t lasm_evaluate_expression(expression_t *expr, hh_darray_t *byte_out){
  if(expr->root == NULL) return ERR; // No expression to evaluate
  // Evaluate the expression tree
}

//-----------------------------------------------------------------------------
uint32_t get_size_of_file(FILE* file){
  uint32_t current = ftell(file);
  fseek(file, 0, SEEK_END);
  uint32_t size = ftell(file);
  fseek(file, current, SEEK_SET);
  return size;
}
//----------------------------------------------------------------------------
uint8_t lasm_put_number_to_file(uint32_t number, FILE *output){
  if(number <= UINT8_MAX) fputc(number, output);
  else if(number <= UINT16_MAX) fwrite(&number, 2, 1, output);
  else if(number <= (1<<24)) fwrite(&number, 3, 1, output);
  else if(number <= UINT32_MAX) fwrite(&number, 4, 1, output);
}
//----------------------------------------------------------------------------
uint8_t lasm_put_bytes_to_file(hh_darray_t* bytes, FILE *output){
  for(size_t i = 0; i < hh_darray_get_fill(bytes); i++){
    uint8_t byte; hh_darray_get(bytes, i, &byte);
    fputc(byte, output);
  }
  return 0;
}
//----------------------------------------------------------------------------
uint8_t lasm_token_to_number(token_t *token, uint32_t *number){
  if(token->id == NUMBER){
      if(strlen(token->text) >= 3){
          if(memcmp(token->text, "0b", 2) == 0){ // If number binary
                *number = strtol(&token->text[2], NULL, 2);	         
                return 0;       
          }else if(memcmp(token->text, "0x", 2) == 0){ // If number hexadecimal
                *number = strtol(&token->text[2], NULL, 16);
                return 0;
          }
      }
        // Else expect decimal
        *number = strtol(token->text, NULL, 10);
  }else{
      print_error_loc(token);
      printf("Expected number but got '%s'\n", token->text);
      return(ERR);
  }
  return 0;    
}
//-----------------------------------------------------------------------------
uint8_t lasm_expect(hh_darray_t *tokens, TOKEN_ID expected){
  if(hh_darray_get_fill(tokens) == 0) return ERR;
  token_t *token = hh_darray_get_reference(tokens, 0);
  if(token->id != expected){
    print_error_loc(token);
    printf("Expected token '%s' but got '%s'\n", token_id_to_string(expected), token->text);
    return ERR;
  }
  hh_darray_pop(tokens, 0, 0);
  return 0;
}
//-----------------------------------------------------------------------------
uint8_t lasm_expect_and_skip(hh_darray_t *tokens, TOKEN_ID expected){
  if(hh_darray_get_fill(tokens) == 0) return ERR;
  token_t *token = hh_darray_get_reference(tokens, 0);
  if(token->id != expected){
    print_error_loc(token);
    printf("Expected token '%s' but got '%s'\n", token_id_to_string(expected), token->text);
    return ERR;
  }
  hh_darray_pop(tokens, 0, 0);
  return 0;
}
