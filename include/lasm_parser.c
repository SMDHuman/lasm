//-----------------------------------------------------------------------------
// lasm_parser.c 14.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "lasm_parser.h"
#include "lasm_assembler.h"

static int32_t tokens_precedence(token_t *token);
static int32_t sbrac_count = 0;
static int32_t rbrac_count = 0;
static int32_t last_low_precedence = 1;

//-----------------------------------------------------------------------------
uint8_t parser_expression(hh_darray_t *tokens, expression_t *expr){
  last_low_precedence = 1;
  hh_darray_init(&expr->expression_tree_buffer, sizeof(expression_tree_t));
  expr->root = parser_expression_right(tokens, expr, 1);
  if(expr->root == NULL) return ERR;
  while(hh_darray_get_item_fill(tokens) > 0 && last_low_precedence > 0){
    //Check if it can continue parsing, else break
    token_t *token = hh_darray_get_reference(tokens, 0);
    if(token->id == NEWLINE || token->id == COLON){
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
    if(token->id == PLUS || token->id == MINUS || token->id == ASTERISK || token->id == SLASH || token->id == INDEX || token->id == DOT){
      hh_darray_pop(tokens, 0, &new_node->token);
      last_low_precedence = tokens_precedence(&new_node->token);
    }else{
      print_error_loc(token);
      printf("Expected an operator (+, -, *, /) but got '%s'\n", token->text);
      return ERR;
    }
    new_node->right = (struct expression_tree_t *)parser_expression_right(tokens, expr, last_low_precedence);
    if(new_node->right == NULL) return ERR;
    expr->root = new_node;
  }
  return 0;
}

//-----------------------------------------------------------------------------
expression_tree_t* parser_expression_right(hh_darray_t *tokens, expression_t *expr, int32_t precedence){
  if(hh_darray_get_fill(tokens) == 0) return 0; // No tokens to parse
  expression_tree_t* expr_tree_out = NULL;
  token_t* token = hh_darray_get_reference(tokens, 0);
  // Check for open round parentheses
  while(token->id == RBRAC_O){
    rbrac_count++;
    hh_darray_pop(tokens, 0, 0); // Consume
  }
  //...
  expression_tree_t *value_node;
  expression_tree_t *op_node;
  if(token->id == WORD || token->id == NUMBER || token->id == STRING_DB || token->id == STRING_SG || token->id == SBRAC_O){
    // Handle simple tokens
    hh_darray_append(&expr->expression_tree_buffer, 0); // create new node
    value_node = hh_darray_get_end_reference(&expr->expression_tree_buffer);
    value_node->left = NULL; // No left child for simple tokens
    value_node->right = NULL; // No right child for simple tokens
    memcpy(&value_node->token, token, sizeof(token_t));
    hh_darray_pop(tokens, 0, 0); // Consume the token
    expr_tree_out = value_node;
    last_low_precedence = 1;
  }else{
    print_error_loc(token);
    printf("Unknown token for parsing '%s'\n", token->text);
    return 0; // ERROR
  }
  while(1){
    // Check if open square paranthases there for indexing
    if(token->id == SBRAC_O){
      sbrac_count++;
      token->id = INDEX;
      token->text[0] = '#';
    }
    // Check for close round parentheses
    else if(token->id == RBRAC_C){
      hh_darray_pop(tokens, 0, 0); // Consume
      rbrac_count--;
    }
    // Check for close square parentheses
    else if(token->id == SBRAC_C){
      if(sbrac_count == 0){
        last_low_precedence = 0;
        return expr_tree_out;
      }
      hh_darray_pop(tokens, 0, 0); // Consume Square Close
      sbrac_count--;
    }
    else{
      break;
    }
  }
  // Check for filling dot
  if(token->id == DOT){
    token_t buffer;
    hh_darray_pop(tokens, 0, &buffer); // Consume Dot
    if(lasm_expect_and_skip(tokens, SBRAC_O) == ERR) return 0; // Expect and skip square bracket open
    hh_darray_push(tokens, 0, &buffer); // Restore Dot
    sbrac_count++;
  }
  // Look for the precedence value of next token
  uint32_t my_precedence = tokens_precedence(token) ;
  my_precedence += (rbrac_count > 0 ? 4 + rbrac_count : 0);
  my_precedence += (sbrac_count > 0 ? 4 + sbrac_count : 0);
  // Compare with the current precedence
  if(my_precedence >= precedence){
    hh_darray_append(&expr->expression_tree_buffer, 0); // create new node
    op_node = hh_darray_get_end_reference(&expr->expression_tree_buffer);
    op_node->left = (struct expression_tree_t *)value_node;
    hh_darray_append(&expr->expression_tree_buffer, 0); // create new nodes right
    op_node->right = hh_darray_get_end_reference(&expr->expression_tree_buffer);
    memcpy(&op_node->token, token, sizeof(token_t));
    hh_darray_pop(tokens, 0, 0); // Consume the token
    op_node->right = (struct expression_tree_t *)parser_expression_right(tokens, expr, my_precedence);
    if(op_node->right == NULL) return 0;
    expr_tree_out = op_node;
  }else{
    last_low_precedence = my_precedence;
  }
  return expr_tree_out;
}

int32_t tokens_precedence(token_t *token){
  if(token->id == PLUS || token->id == MINUS) return 2;
  if(token->id == ASTERISK || token->id == SLASH) return 3;
  return 0; // Default precedence for other tokens
}

//-----------------------------------------------------------------------------
void print_expression_tree(expression_tree_t *root){
  if(root == NULL) return;
  if(root->left != NULL || root->right != NULL) printf("(");
  print_expression_tree((expression_tree_t *)root->left);
  printf(" %s ", root->token.text);
  print_expression_tree((expression_tree_t *)root->right);
  if(root->left != NULL || root->right != NULL) printf(")");
}