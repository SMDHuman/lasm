//-----------------------------------------------------------------------------
// lasm_parser.c 14.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "lasm_parser.h"

static int32_t tokens_precedence(token_t *token);
static int32_t sbrac_count = 0;
static int32_t rbrac_count = 0;

//-----------------------------------------------------------------------------
uint8_t parser_expression(hh_darray_t *tokens, expression_t *expr){
  hh_darray_init(&expr->expression_tree_buffer, sizeof(expression_tree_t));
  if(parser_expression_right(tokens, 1, &expr->expression_tree_buffer, (expression_tree_t *)&expr->root) == ERR) return ERR;
  while(hh_darray_get_item_fill(tokens) > 0){
    //Check if it can continue parsing, else break
    token_t *token = hh_darray_get_reference(tokens, 0);
    if(token->id == NEWLINE){
      if(sbrac_count!=0){
        print_error_loc(token);
        printf("Unmatched square bracket\n");
        return ERR;
      }
      if(rbrac_count!=0){
        print_error_loc(token);
        printf("Unmatched round bracket\n");
        return ERR;
      }
      break;
    }
    //...
    hh_darray_append(&expr->expression_tree_buffer, 0); // create new node
    expression_tree_t *new_node = hh_darray_get_end_reference(&expr->expression_tree_buffer);
    new_node->left = (struct expression_tree_t *)expr->root;
    if(token->id == PLUS || token->id == MINUS || token->id == ASTERISK || token->id == SLASH){
      hh_darray_pop(tokens, 0, &new_node->token);
    }else{
      print_error_loc(token);
      printf("Expected an operator (+, -, *, /) but got '%s'\n", token->text);
      return ERR;
    }
    if(parser_expression_right(tokens, 1, &expr->expression_tree_buffer, (expression_tree_t *)&new_node->right) == ERR) return ERR;
    expr->root = new_node;
  }
  return 0;
}

//-----------------------------------------------------------------------------
uint8_t parser_expression_right(hh_darray_t *tokens, int32_t precedence, hh_darray_t *expression_tree_buffer, expression_tree_t **expr_tree_out){
  if(hh_darray_get_fill(tokens) == 0) return 0; // No tokens to parse
  token_t* token = hh_darray_get_reference(tokens, 0);
  expression_tree_t *value_node;
  expression_tree_t *op_node;
  printf("Token: %s\n", token->text);
  if(token->id == RBRAC_O) {
    rbrac_count++;
    hh_darray_pop(tokens, 0, 0);
  }
  if(token->id == WORD || token->id == NUMBER || token->id == STRING_DB || token->id == SBRAC_O){
    // Handle simple tokens
    hh_darray_append(expression_tree_buffer, 0); // create new node
    value_node = hh_darray_get_end_reference(expression_tree_buffer);
    value_node->left = NULL; // No left child for simple tokens
    value_node->right = NULL; // No right child for simple tokens
    memcpy(&value_node->token, token, sizeof(token_t));
    hh_darray_pop(tokens, 0, &value_node->token); // Consume the token
  }else{
    print_error_loc(token);
    printf("Unknown token for parsing\n");
    return ERR;
  }
  //...
  if(token->id == RBRAC_C) {
    rbrac_count--;
    hh_darray_pop(tokens, 0, 0);
  }
  if(token->id == SBRAC_C) {
    sbrac_count--;
    hh_darray_pop(tokens, 0, 0);
  }
  uint32_t my_precedence = tokens_precedence(token) + (sbrac_count ? 5 : 0)+ (rbrac_count ? 4 : 0);
  if(token->id == SBRAC_O) sbrac_count++;
  //...
  if(my_precedence < precedence){
    (*expr_tree_out) = value_node;
  }else{
    hh_darray_append(expression_tree_buffer, 0); // create new node
    op_node = hh_darray_get_end_reference(expression_tree_buffer);
    op_node->left = value_node;
    hh_darray_append(expression_tree_buffer, 0); // create new nodes right
    op_node->right = hh_darray_get_end_reference(expression_tree_buffer);
    memcpy(&op_node->token, token, sizeof(token_t));
    hh_darray_pop(tokens, 0, 0); // Consume the token
    if(parser_expression_right(tokens, my_precedence, expression_tree_buffer, &op_node->right) == ERR) return ERR;
    (*expr_tree_out) = op_node;
  }
  return 0;
}

int32_t tokens_precedence(token_t *token){
  if(token->id == PLUS || token->id == MINUS) return 2;
  if(token->id == ASTERISK || token->id == SLASH) return 3;
  if(token->id == SBRAC_O) return 4; // Parentheses have the highest precedence
  return 0; // Default precedence for other tokens
}

//-----------------------------------------------------------------------------
void print_expression_tree(expression_tree_t *root){
  if(root == NULL) return;
  printf("(");
  print_expression_tree(root->left);
  printf(" %s ", root->token.text);
  print_expression_tree(root->right);
  printf(")");
}