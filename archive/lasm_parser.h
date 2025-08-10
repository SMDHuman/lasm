//-----------------------------------------------------------------------------
// lasm_parser.h 08.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM_PARSER_H
#define LASM_PARSER_H

#include "lasm_tokenizer.h"
#include "hh_darray.h"

typedef struct {
  uint32_t value;
  uint8_t is_operation; // 1 if it is an operation, 0 if it is a value
  struct expression_t* left;
  struct expression_t* right;
  struct expression_t* next; // Next expression in the list
} expression_t;

typedef struct {
  token_t token; // Token that this constant represents
  expression_t* expression; // Expression that this constant represents
} constant_t;

uint8_t lasm_parser(hh_darray_t* tokens, hh_darray_t* expressions);

#ifdef LASM_PARSER_IMPLEMENTATION
  uint8_t lasm_parser(hh_darray_t* tokens, hh_darray_t* expressions){
    uint32_t head = 0;
    uint32_t address = 0;
    hh_darray_t constants;
    hh_darray_init(&constants, sizeof(constant_t));
    while(1){
      token_t* token = (token_t*)hh_darray_get_reference(tokens, head);
      head++;
      if(token->id == SBRAC_O){
        head--;
        lasm_parse_expressions(tokens, expressions, &head, &constants);
      }
      else if(token->id == NUMBER){
        head--;
        lasm_parse_expressions(tokens, expressions, &head, &constants);
      }
      else if(token->id == WORD){
        token_t *next_token = (token_t*)hh_darray_get_reference(tokens, head);
        head++;
        if(next_token->id == COLON){
          // This is a label
          constant_t constant = {0};
          memcpy(&constant.token, token, sizeof(token_t));
          expression_t expr = {0};
          expr.value = address;
          hh_darray_append(expressions, &expr);
          constant.expression = hh_darray_get_reference(expressions, hh_darray_get_item_fill(expressions) - 1);
          hh_darray_append(&constants, &constant);
        }
        else if(next_token->id == RBRAC_O){
          // This is a vector
          lasm_parse_expressions(tokens, expressions, &head, &constants);
          token_t *next_next_token = (token_t*)hh_darray_get_reference(tokens, head);
          head++;
          if(next_next_token->id == RBRAC_C){
            token_t *next_next_next_token = (token_t*)hh_darray_get_reference(tokens, head);
            head++;
            if(next_next_next_token->id == COLON){
            // This is a label
            constant_t constant = {0};
            memcpy(&constant.token, token, sizeof(token_t));
            hh_darray_append(&constants, &constant);
            }
            else {
              print_error_loc(next_next_next_token);
              printf("Expected colon ':' after vector, but found '%s'\n", next_next_next_token->text);
              return ERR; // Error: expected colon after vector
            }
          }else {
            print_error_loc(token);
            printf("Expected closing square bracket ']', but found '%s'\n", next_next_token->text);
            return ERR; // Error: expected closing square bracket
          }
        }
        else if(next_token->id == PLUS || next_token->id == MINUS ||
                next_token->id == SLASH || next_token->id == ASTERISK){
          // This is an operation
          head--;
          head--;
          lasm_parse_expressions(tokens, expressions, &head, &constants);
        }
        else {
          head--;
          parse_machine_instruction(tokens, expressions, &head, &constants);
        }
      }
      else{
        print_error_loc(token);
        printf("Unexpected token '%s'\n", token->text);
        return ERR; // Error: unexpected token
      }
      //------------------------------------
      // Check for newline after expression
      if(head >= hh_darray_get_item_fill(tokens)) break;
      token_t* token = (token_t*)hh_darray_get_reference(tokens, head);
      head++;
      if(token->id != NEWLINE) {
        print_error_loc(token);
        printf("Expected newline after expression, but found '%s'\n", token->text);
        return ERR; // Error: expected newline
      }
    }
  }
  //-----------------------------------------------------------------------------
  uint8_t lasm_parse_data_section(hh_darray_t* tokens, hh_darray_t* expressions,
                                  hh_darray_t* constants, uint32_t* head){
    lasm_parse_expressions(tokens, expressions, head, constants);
    token_t* token = (token_t*)hh_darray_get_reference(tokens, *head);
    (*head)++;
    if(token->id == DOT){
      token_t* next_token = (token_t*)hh_darray_get_reference(tokens, *head);
      (*head)++;
      if(next_token->id == RBRAC_O){
        lasm_parse_expressions(tokens, expressions, head, constants);
        token_t* next_next_token = (token_t*)hh_darray_get_reference(tokens, *head);
        (*head)++;
        if(next_next_token->id != RBRAC_C){
          print_error_loc(next_next_token);
          printf("Expected closing square bracket ']', but found '%s'\n", next_next_token->text);
          return ERR; // Error: expected closing square bracket
        }
      }
      else{
        print_error_loc(next_token);
        printf("Expected opening square bracket '[', but found '%s'\n", next_token->text);
        return ERR; // Error: expected opening square bracket
      }
    }
    return 0;
  }
    
  //-----------------------------------------------------------------------------
  uint8_t lasm_parse_expressions(hh_darray_t* tokens, hh_darray_t* expressions,
                                  hh_darray_t* constants, uint32_t* head){
    return 0;
  }

  //-----------------------------------------------------------------------------
  uint32_t lasm_parser_length_of_expressions(hh_darray_t* expressions){
    uint32_t length = 1;
    expression_t* expr = (expression_t*)hh_darray_get_reference(expressions, 0);
    while(expr->next != NULL){
      expr = expr->next;
      length++;
    }
    return length;
  }
  //-----------------------------------------------------------------------------
  expression_t* lasm_parser_get_last_expression(hh_darray_t* expressions){
    if(hh_darray_get_item_fill(expressions) == 0) return NULL;
    expression_t* expr = (expression_t*)hh_darray_get_reference(expressions, 0);
    while(expr->next != NULL){
      expr = expr->next;
    }
    return expr;
  }

#endif
#endif