//-----------------------------------------------------------------------------
// lasm_parser.c is part of the LASM (LOTP Assembler) project.
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "lasm_parser.h"
#include "lasm_assembler.h"

static int32_t tokens_precedence(token_t *token);
expression_tree_t* relocate_expression_tree(expression_tree_t *root, expression_t* current_expr, expression_t* target_expr);

//-----------------------------------------------------------------------------
uint8_t parser_expression(hh_darray_t *tokens, expression_t *expr){
  expr->last_low_precedence = 1;
  hh_darray_init(&expr->expression_tree_buffer, sizeof(expression_tree_t));
  expr->root = parser_expression_right(tokens, expr, 1);
  if(expr->root == NULL) return ERR;
  while(hh_darray_get_item_fill(tokens) > 0 && expr->last_low_precedence > 0){
    //Check if it can continue parsing, else break
    token_t *token = hh_darray_get_reference(tokens, 0);
    if(token->id == RBRAC_C){
      break;
    }
    if(token->id == SBRAC_C){
      break;
    }
    if(token->id == NEWLINE){
      break;
    }
    //...
    hh_darray_append(&expr->expression_tree_buffer, 0); // create new node
    expression_tree_t *new_node = hh_darray_get_end_reference(&expr->expression_tree_buffer);
    new_node->left = (struct expression_tree_t *)expr->root;
    if(token->id == PLUS || token->id == MINUS || token->id == ASTERISK || token->id == SLASH || token->id == INDEX || token->id == DOT){
      hh_darray_pop(tokens, 0, &new_node->token);
      expr->last_low_precedence = tokens_precedence(&new_node->token);
    }else{
      print_error_loc(token);
      printf("Expected an operator (+, -, *, /) but got '%s'\n", token->text);
      return ERR;
    }
    new_node->right = (struct expression_tree_t *)parser_expression_right(tokens, expr, expr->last_low_precedence);
    if(new_node->right == NULL) return ERR;
    expr->root = new_node;
  }
  print_expression_tree(expr->root);printf("\n");
  return 0;
}

//-----------------------------------------------------------------------------
expression_tree_t* parser_expression_right(hh_darray_t *tokens, expression_t *expr, int32_t precedence){
  if(hh_darray_get_fill(tokens) == 0) return NULL; // No tokens to parse
  expression_tree_t* expr_tree_out = NULL;
  token_t* token = hh_darray_get_reference(tokens, 0);
  //...
  expression_tree_t *value_node;
  expression_tree_t *op_node;
  if(token->id == WORD || token->id == NUMBER || token->id == STRING_DB || token->id == STRING_SG){
    // Handle simple tokens
    hh_darray_append(&expr->expression_tree_buffer, 0); // create new node
    value_node = hh_darray_get_end_reference(&expr->expression_tree_buffer);
    value_node->left = NULL; // No left child for simple tokens
    value_node->right = NULL; // No right child for simple tokens
    memcpy(&value_node->token, token, sizeof(token_t));
    hh_darray_pop(tokens, 0, 0); // Consume the token
    expr_tree_out = value_node;
    expr->last_low_precedence = 1;
  }
  else if(token->id == RBRAC_O || token->id == SBRAC_O){
    // Handle round brackets
    hh_darray_pop(tokens, 0, 0); // Consume '('
    expression_t isolated_expr;
    if(parser_expression(tokens, &isolated_expr) == ERR) return NULL;
    value_node = relocate_expression_tree(isolated_expr.root, &isolated_expr, expr);
    expr_tree_out = value_node;
    hh_darray_pop(tokens, 0, 0); // Consume ')'
    expr->last_low_precedence = 1;
    hh_darray_deinit(&isolated_expr.expression_tree_buffer);
  }
  else{
    print_error_loc(token);
    printf("Unknown token for parsing '%s'\n", token->text);
    return NULL; // ERROR
  }
  // Check if open square paranthases there for indexing
  if(token->id == SBRAC_O){
    token_t buffer; hh_darray_get(tokens, 0, &buffer); // Get the token
    token->id = INDEX;
    token->text[0] = '#';
    hh_darray_push(tokens, 1, &buffer); // push the token
  }
  // Check if it is a right bracket
  if(token->id == RBRAC_C || token->id == SBRAC_C){
    return expr_tree_out; // Return the expression tree
  }
  // Check for filling dot
  if(token->id == DOT){
    token_t *next = hh_darray_get_reference(tokens, 1);
    if(next->id != SBRAC_O){
      print_error_loc(next);
      printf("Expected '[' after '.'\n");
      return NULL;
    }
  }
  // Look for the precedence value of next token
  uint32_t my_precedence = tokens_precedence(token) ;
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
    if(op_node->right == NULL) return NULL;
    expr_tree_out = op_node;
  }else{
    expr->last_low_precedence = my_precedence;
  }
  return expr_tree_out;
}

int32_t tokens_precedence(token_t *token){
  if(token->id == PLUS || token->id == MINUS) return 2;
  if(token->id == ASTERISK || token->id == SLASH) return 3;
  if(token->id == INDEX || token->id == DOT) return 4;
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
//-----------------------------------------------------------------------------
expression_tree_t* relocate_expression_tree(expression_tree_t *root, expression_t* current_expr, expression_t* target_expr){
  hh_darray_append(&target_expr->expression_tree_buffer, root);
  root = (expression_tree_t *)hh_darray_get_end_reference(&target_expr->expression_tree_buffer);
  if(root->left != NULL){
    root->left = (struct expression_tree_t *)relocate_expression_tree((expression_tree_t *)root->left, current_expr, target_expr);
  }
  if(root->right != NULL){
    root->right = (struct expression_tree_t *)relocate_expression_tree((expression_tree_t *)root->right, current_expr, target_expr);
  }
  return root;
}