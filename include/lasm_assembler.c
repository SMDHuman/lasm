//-----------------------------------------------------------------------------
// lasm_assembler.c 10.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "lasm_assembler.h"
#include "lasm_tokenizer.h"
#include "lasm_namespace.h"

//-----------------------------------------------------------------------------
assembler_t lasm_assembler;

static uint32_t get_size_of_file(FILE* file);
static uint8_t is_lineend_token_id(hh_darray_t *tokens, TOKEN_ID id);

//-----------------------------------------------------------------------------
// Function to assemble the code
uint8_t lasm_assemble(hh_darray_t *tokens, FILE *output){
  //...
  lasm_assembler.current_namespace = &global_space;
  lasm_assembler.unnamed_namespace_index = 0;
  lasm_assembler.tokens = tokens;
  lasm_assembler.output_file = output;
  hh_darray_init(&lasm_assembler.backward_patches, sizeof(backward_patch_t));
  //...
  token_t *token = hh_darray_get_reference(tokens, 0);
  while(hh_darray_get_fill(tokens) > 0){
    if(token->id == WORD){
      // Handle word token
      token_t *next_token = hh_darray_get_reference(tokens, 1);
      label_t *label = lasm_find_label_in_namespace(lasm_assembler.current_namespace, token->text);
      if(label != NULL){
        if(is_lineend_token_id(tokens, COLON)){
          if(label->is_vector){
            // Handle vector token
            hh_darray_pop(tokens, 0, 0); // Consume label token
            hh_darray_pop(tokens, 0, 0); // Consume open bracket
            hh_darray_t out_bytes;
            hh_darray_init(&out_bytes, 1);
            if(lasm_parse_expression(tokens, &out_bytes, 0, -1) == ERR) return ERR;
            for(int i = 0; i < hh_darray_get_fill(&out_bytes); i++){
              uint8_t *byte = hh_darray_get_reference(&out_bytes, i);
              label->value += (*byte) << (i * 8);
            }
            hh_darray_pop(tokens, 0, 0); // Consume close bracket
            fseek(output, label->value, SEEK_SET);
            label->is_valid = 1;
            hh_darray_pop(tokens, 0, 0); // Consume colon
            hh_darray_deinit(&out_bytes);
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
          hh_darray_t out_bytes;
          hh_darray_init(&out_bytes, 1);
          if(lasm_parse_expression(tokens, &out_bytes, 1, -1) == ERR) return ERR;
          lasm_put_bytes_to_file(&out_bytes, output);
          hh_darray_deinit(&out_bytes);
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
      hh_darray_t out_bytes;
      hh_darray_init(&out_bytes, 1);
      if(lasm_parse_expression(tokens, &out_bytes, 1, -1) == ERR) return ERR;
      lasm_put_bytes_to_file(&out_bytes, output);
      hh_darray_deinit(&out_bytes);
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
  // =====================
  // Backwards patches
  for(uint32_t i = 0; i < hh_darray_get_item_fill(&lasm_assembler.backward_patches); i++){
    backward_patch_t *patch = hh_darray_get_reference(&lasm_assembler.backward_patches, i);
    if(patch->label->is_valid){
      fseek(output, patch->offset, SEEK_SET);
      uint32_t label_value; fread(&label_value, patch->size, 1, output);
      if(patch->operation == PLUS) label_value += patch->label->value;
      else if(patch->operation == MINUS) label_value -= patch->label->value;
      else{
        print_error_loc(&patch->label->name);
        printf("Unsupported patch operation for label '%s'\n", patch->label->name.text);
        return ERR;
      }
      fseek(output, patch->offset, SEEK_SET);
      fwrite(&label_value, patch->size, 1, output);
    }else{
      print_error_loc(&patch->label->name);
      printf("Label '%s' can't be validated. Unsupported feature.\n", patch->label->name.text);
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
// Checks if the token before first newline equal to given id
uint8_t is_lineend_token_id(hh_darray_t *tokens, TOKEN_ID id){
  token_t *token = hh_darray_get_reference(tokens, 0);
  uint32_t i;
  for(i = 0; token->id != NEWLINE; i++){
    token = hh_darray_get_reference(tokens, i);
  }
  token = hh_darray_get_reference(tokens, i-2);
  return token->id == id ? 1 : 0;
}
//-----------------------------------------------------------------------------
uint8_t lasm_parse_expression(hh_darray_t *tokens, hh_darray_t *out_bytes, uint8_t stash_bwp, uint32_t max_size){
  token_t *token = hh_darray_get_reference(tokens, 0);
  uint32_t number = 0;
  uint32_t size = 0;
  while(hh_darray_get_fill(tokens) > 0){
    if(token->id == NUMBER){
      // Handle number token
      if(lasm_eval_token(token, (uint32_t*)&number, &size, PLUS, stash_bwp, max_size) == ERR) return ERR;
      if(size > 0){
        number = number & ((1 << (size * 8)) - 1);
      }
      hh_darray_pop(tokens, 0, 0);
    }
    else if(token->id == WORD){
      // Handle word token
      if(lasm_eval_token(token, (uint32_t*)&number, &size, PLUS, stash_bwp, max_size) == ERR) return ERR;
      if(size > 0){
        number = number & ((1 << (size * 8)) - 1);
      }
      hh_darray_pop(tokens, 0, 0);
    }
    else if(token->id == PLUS){
      hh_darray_pop(tokens, 0, 0); // Consume plus token
      uint32_t right_number;
      if(lasm_eval_token(token, &right_number, &size, PLUS, stash_bwp, max_size) == ERR) return ERR;
      number = (uint32_t)number + right_number;
      if(size > 0){
        number = number & ((1 << (size * 8)) - 1);
      }
      hh_darray_pop(tokens, 0, 0); // Consume right operand
    }
    else if(token->id == MINUS){
      hh_darray_pop(tokens, 0, 0); // Consume plus token
      uint32_t right_number;
      if(lasm_eval_token(token, &right_number, &size, MINUS, stash_bwp, max_size) == ERR) return ERR;
      number = (uint32_t)number - right_number;
      if(size > 0){
        number = number & ((1 << (size * 8)) - 1);
      }
      hh_darray_pop(tokens, 0, 0); // Consume right operand
    }
    else if(token->id == DOT){
      hh_darray_pop(tokens, 0, 0); // Consume dot
      if(token->id == SBRAC_O){
        hh_darray_pop(tokens, 0, 0); // Consume left bracket
        hh_darray_t size_value; hh_darray_init(&size_value, 1);
        if(lasm_parse_expression(tokens, &size_value, 0, -1) == ERR) return ERR;
        for(uint8_t i = 0; i < hh_darray_get_fill(&size_value); i++){
          uint8_t *byte = hh_darray_get_reference(&size_value, i);
          size += (*byte) << (i * 8);
        }
        size = size > max_size ? max_size : size;
        if(token->id != SBRAC_C){
          print_error_loc(token);
          printf("Expected a right bracket '}' but got '%s'\n", token->text);
          return ERR;
        }
        hh_darray_pop(tokens, 0, 0); // Consume right bracket
        hh_darray_deinit(&size_value);
      }
    }
    else{
      break;
      //print_error_loc(token);
      //printf("Unexpected token while parsing expression: '%s'\n", token->text);
      //return ERR;
    }
  }
  if(number == 0 && size == 0){
    hh_darray_append(out_bytes, 0);
  }
  while(number > 0){
    hh_darray_append(out_bytes, &number);
    number = number >> 8;
    if(size > 0) size--;
  }
  while(size > 0){
    hh_darray_append(out_bytes, 0);
    size--;
  }
  return 0;
}
//-----------------------------------------------------------------------------
uint8_t lasm_eval_token(token_t *token, uint32_t *out_number, uint32_t *size, TOKEN_ID operation, uint8_t stash_bwp, uint32_t max_size){
  if(token->id == NUMBER){
    // Handle number token
    uint32_t number;
    if(lasm_token_to_number(token, &number) == ERR) return ERR;
    (*out_number) = number;
    return 0;
  }
  else if(token->id == WORD){
    // Handle word token
    label_t *label = lasm_find_label_in_namespace(lasm_assembler.current_namespace, token->text);
    if(label != NULL){
      if(!label->is_vector){
        (*size) = DEFAULT_ADDRESSING_SIZE < max_size ? DEFAULT_ADDRESSING_SIZE : max_size;
      }
      if(label->is_valid){
        // Handle vector token
        (*out_number) = label->value;
        return 0;
      }
      else{
        if(stash_bwp){
          backward_patch_t patch = {0};
          patch.size = DEFAULT_ADDRESSING_SIZE < max_size ? DEFAULT_ADDRESSING_SIZE : max_size;
          patch.offset = ftell(lasm_assembler.output_file);
          patch.operation = operation;
          patch.label = label;
          hh_darray_append(&lasm_assembler.backward_patches, &patch);
          (*out_number) = 0;
          (*size) = DEFAULT_ADDRESSING_SIZE < max_size ? DEFAULT_ADDRESSING_SIZE : max_size;
          return 0;
        }else{
          print_error_loc(token);
          printf("Label '%s' must be a vector address.\n", token->text);
          return ERR;
        }
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

