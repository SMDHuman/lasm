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
const char TAG[] = "[ASMB]";

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
            hh_bigint_t number; hh_bigint_init(&number, 0);
            if(lasm_evaluate_expression_tree(expression.root, &number) == ERR) return ERR;
            if(number.size > 4){
              printf(TAG);
              print_error_loc(token);
              printf("Vector expression exceeds 4 bytes\n");
            }
            if(number.size == 1) label->value = (*(uint32_t *)number.data) & 0xff;
            if(number.size == 2) label->value = (*(uint32_t *)number.data) & 0xffff;
            if(number.size == 3) label->value = (*(uint32_t *)number.data) & 0xffffff;
            if(number.size == 4) label->value = (*(uint32_t *)number.data) & 0xffffffff;
            label->is_valid = 1;
            //...
            if(lasm_expect_and_skip(tokens, SBRAC_C) == ERR) return ERR;
            fseek(output, label->value, SEEK_SET);
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
          hh_bigint_t value; hh_bigint_init(&value, 0);
          if(lasm_evaluate_expression_tree(expression.root, &value) == ERR) return ERR;
          fwrite(value.data, value.size, 1, lasm_assembler.output_file);
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
    else if(token->id == NUMBER || token->id == STRING_DB || token->id == RBRAC_C){
      //TODO: Implement the evaluation of expressions
      expression_t expression;
      if(parser_expression(tokens, &expression) == ERR) return ERR;
      hh_bigint_t value; hh_bigint_init(&value, 0);
      if(lasm_evaluate_expression_tree(expression.root, &value) == ERR) return ERR;
      fwrite(value.data, value.size, 1, lasm_assembler.output_file);
    }
    // Vector with no name
    else if(token->id == SBRAC_O){
      if(is_lineend_token_id(tokens, 0, COLON)){
        // Handle vector token
        hh_darray_pop(tokens, 0, 0); // Consume open bracket
        //...
        //TODO: Implement the evaluation of expressions
        expression_t expression;
        if(parser_expression(tokens, &expression) == ERR) return ERR;
        print_expression_tree(expression.root);printf("\n");
        hh_bigint_t value; hh_bigint_init(&value, 0);
        lasm_evaluate_expression_tree(expression.root, &value);
        hh_bigint_print_hex(&value);
        // Evaluate expression
        //uint32_t value;// = lasm_evaluate_expression(&expression, output);
        ////...
        if(lasm_expect_and_skip(tokens, SBRAC_C) == ERR) return ERR;
        //fseek(output, value, SEEK_SET);
        if(lasm_expect_and_skip(tokens, COLON) == ERR) return ERR;
      }else{
        printf(TAG);
        print_error_loc(token);
        printf("Vector must be ending with ':'\n");
        return ERR;
      }
    }
    else{
      printf(TAG);
      print_error_loc(token);
      printf("Unexpected token: %s\n", token->text);
      return ERR;
    }
    // Expect newline
    if(token->id == NEWLINE){
      hh_darray_pop(tokens, 0, 0);
    }else {
      printf(TAG);
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
uint8_t lasm_token_to_number(token_t *token, hh_bigint_t *number){
  if(token->id == NUMBER){
    // Convert token text to bigint
    if(hh_bigint_convert_from_string(number, token->text) == ERR){
      printf(TAG);
      print_error_loc(token);
      printf("Invalid number format: '%s'\n", token->text);
      return(ERR);
    }
  }else{
      printf(TAG);
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
    printf(TAG);
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
    printf(TAG);
    print_error_loc(token);
    printf("Expected token '%s' but got '%s'\n", token_id_to_string(expected), token->text);
    return ERR;
  }
  hh_darray_pop(tokens, 0, 0);
  return 0;
}

//-----------------------------------------------------------------------------
uint8_t lasm_evaluate_expression_tree(expression_tree_t *node, hh_bigint_t *number){
  if(node == NULL) return 0; // No expression to evaluate
  if(node->left != NULL && node->right != NULL) {
    hh_bigint_t left_number; hh_bigint_init(&left_number, 0);
    lasm_evaluate_expression_tree((expression_tree_t*)node->left, &left_number); // Evaluate left subtree
    hh_bigint_t right_number; hh_bigint_init(&right_number, 0);
    lasm_evaluate_expression_tree((expression_tree_t*)node->right, &right_number); // Evaluate right subtree
    size_t biggest_size = (left_number.size > right_number.size ? left_number.size : right_number.size);
    if(node->token.id == PLUS) {
      hh_bigint_add(&left_number, &right_number, number);
    } else if(node->token.id == MINUS) {
      hh_bigint_subtract(&left_number, &right_number, number);
    } else if(node->token.id == ASTERISK) {
      hh_bigint_multiply(&left_number, &right_number, number);
    } else if(node->token.id == DOT){
      size_t *size = (size_t*)right_number.data;
      if(*size == 0){
        hh_bigint_set_zero(number);
      }else{
        hh_bigint_resize(&left_number, *size);
        hh_bigint_copy(number, &left_number);
      }
    }else if(node->token.id == INDEX){
      if(right_number.sign == 1){
        printf(TAG);
        print_error_loc(&node->token);
        printf("Indexing with negative number is not allowed: '%s'\n", node->token.text);
        return ERR;
      }
      size_t *index = (size_t*)right_number.data;
      uint8_t num = hh_bigint_get_at(&left_number, *index);
      hh_bigint_set_zero(number); hh_bigint_set_at(number, num, 0);
    }
    else{
      printf(TAG);
      print_error_loc(&node->token);
      printf("Unsupported operation: '%s'\n", node->token.text);
      return ERR;
    }
    if(number->size < biggest_size){
      if(hh_bigint_resize(number, biggest_size) == ERR) return ERR;
    }
  }else if(node->token.id == NUMBER){
    if(hh_bigint_convert_from_string(number, node->token.text) == ERR) return ERR;
  }else if(node->token.id == STRING_DB){
    hh_bigint_resize(number, strlen(node->token.text));
    memcpy(number->data, node->token.text, strlen(node->token.text));
  }else if(node->token.id == WORD){
    label_t *label = lasm_find_label_in_namespace(lasm_assembler.current_namespace, node->token.text);
    if(label == NULL){
      printf(TAG);
      print_error_loc(&node->token);
      printf("Undefined label: '%s'\n", node->token.text);
      return ERR;
    }
    if(label->is_valid){
      // Handle valid label
      hh_bigint_set_uint32(number, label->value);
      if(label->is_vector){
        hh_bigint_normalize(number);
      }else{
        hh_bigint_resize(number, DEFAULT_ADDRESSING_SIZE);
      }
      return 0;
    }else{
      printf(TAG);
      print_error_loc(&node->token);
      printf("Invalid label: '%s'\n", node->token.text);
      return ERR;
    }
  }
}