//-----------------------------------------------------------------------------
// lasm_assembler.c 10.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "lasm_assembler.h"
#include "lasm_tokenizer.h"
#include "lasm_namespace.h"

//-----------------------------------------------------------------------------
assembler_config_t lasm_config;
hh_darray_t backward_patches; // Patches to apply after first pass
static namespace_t *current_namespace = &global_space;
static uint32_t unnamed_namespace_index = 0;

static uint32_t get_size_of_file(FILE* file);

//-----------------------------------------------------------------------------
// Function to assemble the code
uint8_t lasm_assemble(hh_darray_t *tokens, FILE *output){
  token_t *token = hh_darray_get_reference(tokens, 0);
  while(hh_darray_get_fill(tokens) > 0){
    if(token->id == WORD){
      // Handle word token
      token_t *next_token = hh_darray_get_reference(tokens, 1);
      label_t *label = lasm_find_label_in_namespace(current_namespace, token->text);
      if(label != NULL){
        if(!label->is_valid){
          if(label->is_vector){
            // Handle vector token
            hh_darray_pop(tokens, 0, 0); // Consume label token
            hh_darray_pop(tokens, 0, 0); // Consume open bracket
            if(lasm_eval_token(token, &label->value) == ERR) return ERR;
            hh_darray_pop(tokens, 0, 0); // Consume number
            hh_darray_pop(tokens, 0, 0); // Consume close bracket
            fseek(output, label->value, SEEK_SET);
            label->is_valid = 1;
            hh_darray_pop(tokens, 0, 0); // Consume colon
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
          if(lasm_parse_expression(tokens, output) == ERR) return ERR;
        }
      }
      else if(next_token->id == CBRAC_O){
        // Enter namespace with name
        current_namespace = lasm_find_namespace_from_all(token->text);
        if (current_namespace == NULL) {
          print_error_loc(token);
          printf("Unknown namespace: %s\n", token->text);
          return ERR;
        }
        hh_darray_pop(tokens, 0, 0); // Consume label token
        hh_darray_pop(tokens, 0, 0); // Consume curly brace open
      }
    }
    else if(token->id == CBRAC_O){
      // Handle '{' token
      char index_str[16];
      sprintf(index_str, "__unnamed%d__", unnamed_namespace_index++);
      current_namespace = lasm_find_namespace_from_all(index_str);
      hh_darray_pop(tokens, 0, 0); // Consume curly brace open
    }
    else if(token->id == CBRAC_C){
      // Handle '}' token
      current_namespace = (namespace_t *)current_namespace->parent;
      hh_darray_pop(tokens, 0, 0); // Consume curly brace close
    }
    else if(token->id == NUMBER){
      if(lasm_parse_expression(tokens, output) == ERR) return ERR;
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
    // Expect newline
    if(token->id == NEWLINE){
      hh_darray_pop(tokens, 0, 0);
    }else {
      print_error_loc(token);
      printf("Expected newline but got '%s'\n", token->text);
      return ERR;
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------
uint32_t get_size_of_file(FILE* file){
  uint32_t current = ftell(file);
  fseek(file, 0, SEEK_END);
  uint32_t size = ftell(file);
  fseek(file, current, SEEK_SET);
  return size;
}

//-----------------------------------------------------------------------------
uint8_t lasm_parse_expression(hh_darray_t *tokens, FILE *output){
  token_t *token = hh_darray_get_reference(tokens, 0);
  int64_t number = -1;
  while(hh_darray_get_fill(tokens) > 0){
    if(token->id == NUMBER){
      // Handle number token
      if(lasm_eval_token(token, (uint32_t*)&number) == ERR) return ERR;
      hh_darray_pop(tokens, 0, 0);
    }
    else if(token->id == WORD){
      // Handle word token
      if(lasm_eval_token(token, (uint32_t*)&number) == ERR) return ERR;
      hh_darray_pop(tokens, 0, 0);
    }
    else if(token->id == PLUS){
      hh_darray_pop(tokens, 0, 0); // Consume plus token
      uint32_t right_number;
      if(lasm_eval_token(token, &right_number) == ERR) return ERR;
      number = (uint32_t)number + right_number;
      hh_darray_pop(tokens, 0, 0); // Consume right operand
    }
    else if(token->id == MINUS){
      hh_darray_pop(tokens, 0, 0); // Consume plus token
      uint32_t right_number;
      if(lasm_eval_token(token, &right_number) == ERR) return ERR;
      number = (uint32_t)number - right_number;
      hh_darray_pop(tokens, 0, 0); // Consume right operand
    }
    else{
      break;
      //print_error_loc(token);
      //printf("Unexpected token while parsing expression: '%s'\n", token->text);
      //return ERR;
    }
  }
  if(number != -1) lasm_put_number_to_file(number, output);
  return 0;
}
//-----------------------------------------------------------------------------
uint8_t lasm_eval_token(token_t *token, uint32_t *out_number){
  if(token->id == NUMBER){
    // Handle number token
    uint32_t number;
    if(lasm_token_to_number(token, &number) == ERR) return ERR;
    (*out_number) = number;
    return 0;
  }
  else if(token->id == WORD){
    // Handle word token
    label_t *label = lasm_find_label_in_namespace(current_namespace, token->text);
    if(label != NULL){
      if(label->is_valid){
        // Handle vector token
        (*out_number) = label->value;
        return 0;
      }
      else{
        print_error_loc(token);
        printf("Label '%s' is not valid. Unsupported feature.\n", token->text);
        return ERR;
      }
    }
  }
  else{
    print_error_loc(token);
    printf("Unexpected token while evaluating expression: %s\n", token->text);
    return ERR;
  }
}
//----------------------------------------------------------------------------
uint8_t lasm_put_number_to_file(uint32_t number, FILE *output){
  if(number <= UINT8_MAX) fputc(number, output);
  else if(number <= UINT16_MAX) fwrite(&number, 2, 1, output);
  else if(number <= (1<<24)) fwrite(&number, 3, 1, output);
  else if(number <= UINT32_MAX) fwrite(&number, 4, 1, output);
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

