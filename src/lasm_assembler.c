//-----------------------------------------------------------------------------
// lasm_assembler.c 14.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "lasm_assembler.h"
#include "lasm_tokenizer.h"
#include "lasm_parser.h"

//-----------------------------------------------------------------------------
assembler_t lasm_assembler;

static const char TAG[] = "[ASMB]";

//-----------------------------------------------------------------------------
// Function to assemble the code
uint8_t lasm_assemble(hh_darray_t *tokens, FILE *output){
  // Initialize global namespace
  lasm_assembler.global_namespace = (namespace_t){0};
  token_t* first_token = hh_darray_get_reference(tokens, 0);
  memcpy(&lasm_assembler.global_namespace.name, first_token, sizeof(token_t));
  memset(lasm_assembler.global_namespace.name.text, 0, MAX_TOKEN_SIZE);
  strcat(lasm_assembler.global_namespace.name.text, "__global__");
  lasm_assembler.global_namespace.name.id = 0;
  lasm_assembler.global_namespace.level = 0;
  lasm_assembler.global_namespace.constant = 1;
  hh_darray_init(&lasm_assembler.global_namespace.childs, sizeof(namespace_t));
  hh_darray_init(&lasm_assembler.global_namespace.labels, sizeof(label_t));
  lasm_assembler.current_namespace = &lasm_assembler.global_namespace;
  // Initialize other properties of assembler
  lasm_assembler.unnamed_namespace_count = 0;
  lasm_assembler.tokens = tokens;
  lasm_assembler.output_file = output;
  hh_darray_init(&lasm_assembler.backward_patches, sizeof(expression_t));
  //...
  token_t *token = hh_darray_get_reference(tokens, 0);
  while(hh_darray_get_fill(tokens) > 0){
    if(token->id == WORD){
      // Handle word token
      token_t *next_token = hh_darray_get_reference(tokens, 1);
      if(next_token->id == CBRAC_O){
        namespace_t temp = {.constant = 1,
                            .level = lasm_assembler.current_namespace->level + 1,
                            .parent = lasm_assembler.current_namespace,
                            .name = *token};
        hh_darray_append(&lasm_assembler.current_namespace->childs, &temp);
        lasm_assembler.current_namespace = hh_darray_get_end_reference(&lasm_assembler.current_namespace->childs);
        hh_darray_init(&lasm_assembler.current_namespace->childs, sizeof(namespace_t));
        hh_darray_init(&lasm_assembler.current_namespace->labels, sizeof(label_t));
        hh_darray_pop(tokens, 0, 0); // Consume name
        hh_darray_pop(tokens, 0, 0); // Consume curly brace open
      }
      else if(lasm_is_lineend_id(tokens, 0, COLON)){
        hh_darray_append(&lasm_assembler.current_namespace->labels, 0); 
        label_t* new_label = hh_darray_get_end_reference(&lasm_assembler.current_namespace->labels);
        new_label->name = *token;
        if(next_token->id == SBRAC_O){
          new_label->is_vector = 1;
          hh_darray_pop(tokens, 0, 0); // Consume name
          hh_darray_pop(tokens, 0, 0); // Consume square brace open
          if(parser_expression(tokens, &new_label->expression) == ERR) return ERR;
          if(lasm_expect_and_skip_id(tokens, SBRAC_C) == ERR) return ERR;
          if(lasm_expect_and_skip_id(tokens, COLON) == ERR) return ERR;
        }
        else{
          hh_darray_pop(tokens, 0, 0); // Consume name
          if(lasm_expect_and_skip_id(tokens, COLON) == ERR) return ERR;
          new_label->is_vector = 0;
          new_label->value = lasm_get_file_size();
          new_label->is_evaluated = 1;
        }
      }
      else{
        label_t* label = lasm_find_label_reachable_namespace(lasm_assembler.current_namespace, token->text);
        if(label){
          print_expression_tree(label->expression.root); printf("\n");
          hh_darray_pop(tokens, 0, 0); // Consume token
        }
        else{
          if(lasm_assembler.machine_assemble() == ERR) return ERR;
        }
      }
    }
    else if(token->id == CBRAC_O){
      // Handle '{' token
      namespace_t temp = {.constant = 0,
                          .level = lasm_assembler.current_namespace->level + 1,
                          .parent = lasm_assembler.current_namespace,
                          .name = *token};
      hh_darray_append(&lasm_assembler.current_namespace->childs, &temp);
      lasm_assembler.current_namespace = hh_darray_get_end_reference(&lasm_assembler.current_namespace->childs);
      hh_darray_init(&lasm_assembler.current_namespace->childs, sizeof(namespace_t));
      hh_darray_init(&lasm_assembler.current_namespace->labels, sizeof(label_t));
      hh_darray_pop(tokens, 0, 0); // Consume curly brace open
    }
    else if(token->id == CBRAC_C){
      // Handle '}' token
      namespace_t* upper_ns = (namespace_t *)lasm_assembler.current_namespace->parent;
      if(!lasm_assembler.current_namespace->constant){
        hh_darray_deinit(&lasm_assembler.current_namespace->childs);
        hh_darray_deinit(&lasm_assembler.current_namespace->labels);
        hh_darray_remove_reference(&((namespace_t *)lasm_assembler.current_namespace->parent)->childs, lasm_assembler.current_namespace);
      }
      lasm_assembler.current_namespace = upper_ns;
      hh_darray_pop(tokens, 0, 0); // Consume curly brace close
    }
    else if(token->id == NUMBER || token->id == STRING_DB || token->id == RBRAC_C){
      //TODO: Implement the evaluation of expressions
      expression_t expression;
      if(parser_expression(tokens, &expression) == ERR) return ERR;
      hh_bigint_t value; hh_bigint_init(&value, 0);
      if(lasm_evaluate_expression_tree(expression.root, &value) == ERR) return ERR;
      fwrite(value.data, value.size, 1, lasm_assembler.output_file);
      parser_expression_deinit(&expression);
    }
    // Vector with no name
    else if(token->id == SBRAC_O){
      if(lasm_is_lineend_id(tokens, 0, COLON)){
        // Handle vector token
        hh_darray_pop(tokens, 0, 0); // Consume open bracket
        //...
        //TODO: Implement the evaluation of expressions
        expression_t expression;
        if(parser_expression(tokens, &expression) == ERR) return ERR;
        print_expression_tree(expression.root);printf("\n");
        hh_bigint_t value; hh_bigint_init(&value, 0);
        // Evaluate expression
        if(lasm_evaluate_expression_tree(expression.root, &value) == ERR) return ERR;
        parser_expression_deinit(&expression);
        //uint32_t value;// = lasm_evaluate_expression(&expression, output);
        ////...
        if(lasm_expect_and_skip_id(tokens, SBRAC_C) == ERR) return ERR;
        //fseek(output, value, SEEK_SET);
        if(lasm_expect_and_skip_id(tokens, COLON) == ERR) return ERR;
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
size_t lasm_get_file_size(){
  size_t current = ftell(lasm_assembler.output_file);
  fseek(lasm_assembler.output_file, 0, SEEK_END);
  size_t size = ftell(lasm_assembler.output_file);
  fseek(lasm_assembler.output_file, current, SEEK_SET);
  return size;
}

//----------------------------------------------------------------------------
size_t lasm_get_file_cursor(){
  return ftell(lasm_assembler.output_file);
}

//----------------------------------------------------------------------------
uint8_t lasm_put_number_to_file(uint32_t number, FILE *output){
  if(number <= UINT8_MAX) fputc(number, output);
  else if(number <= UINT16_MAX) fwrite(&number, 2, 1, output);
  else if(number <= (1<<24)) fwrite(&number, 3, 1, output);
  else if(number <= UINT32_MAX) fwrite(&number, 4, 1, output);
  return 0;
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
uint8_t lasm_expect_id(hh_darray_t *tokens, TOKEN_ID expected){
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
uint8_t lasm_expect_and_skip_id(hh_darray_t *tokens, TOKEN_ID expected){
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
uint8_t lasm_is_lineend_id(hh_darray_t *tokens, uint32_t index, TOKEN_ID id){
  token_t *token = hh_darray_get_reference(tokens, index);
  uint32_t i;
  for(i = index + 1; token->id != NEWLINE; i++){
    token = hh_darray_get_reference(tokens, i);
  }
  token = hh_darray_get_reference(tokens, i-2);
  return token->id == id ? 1 : 0;
}
//-----------------------------------------------------------------------------
uint8_t lasm_is_lineend_text(hh_darray_t *tokens, uint32_t index, const char* text){
  token_t *token = hh_darray_get_reference(tokens, index);
  uint32_t i;
  for(i = index + 1; token->id != NEWLINE; i++){
    token = hh_darray_get_reference(tokens, i);
  }
  token = hh_darray_get_reference(tokens, i-2);
  return strcmp(token->text, text) == 0 ? 1 : 0;
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
    // TODO: Implement WORD token handling
    printf(TAG);
    print_error_loc(&node->token);
    printf("Invalid label: '%s'\n", node->token.text);
    return ERR;
  }
  return 0;
}
//-----------------------------------------------------------------------------
label_t* lasm_find_label_reachable_namespace(namespace_t* namespace, const char* name){
    // Check if name contains dot
  if(strchr(name, '.') != NULL) {
    // Split the name into base and sub
    char base[MAX_TOKEN_SIZE];
    char sub[MAX_TOKEN_SIZE];
    for(int i = 0; i < MAX_TOKEN_SIZE; i++) {
      if(name[i] == '.') {
        base[i] = '\0';
        strncpy(sub, &name[i + 1], MAX_TOKEN_SIZE - i - 1);
        break;
      }
      base[i] = name[i];
    }
    for(size_t i = 0; i < hh_darray_get_item_fill(&namespace->childs); i++){
      namespace_t *ns = hh_darray_get_reference(&namespace->childs, i);
      if(strcmp(ns->name.text, base) == 0){
        label_t *lower_label = lasm_find_label_reachable_namespace(ns, sub);
        if(lower_label) return lower_label;
      }
    }
  }
  for(size_t i = 0; i < hh_darray_get_item_fill(&namespace->labels); i++){
    label_t *label = hh_darray_get_reference(&namespace->labels, i);
    if(strcmp(label->name.text, name) == 0) return label;
  }
  if(namespace->parent != NULL){
    return lasm_find_label_reachable_namespace(namespace->parent, name);
  }
  return NULL;
}

//-----------------------------------------------------------------------------
void lasm_export_json_namespace(namespace_t* ns, FILE* file, uint8_t indent_level){
  static char indent[256]; memset(indent, ' ', 256);
  static const uint8_t indent_size = 2;
  // Opening brace
  fwrite(indent, 1, indent_level*indent_size, file);
  fprintf(file, "{\n");
  // Export namespace information
  fwrite(indent, 1, (indent_level + 1)*indent_size, file);
  fprintf(file, "\"name\": \"%s\",\n", ns->name.text);
  // Export constant property
  fwrite(indent, 1, (indent_level + 1)*indent_size, file);
  fprintf(file, "\"constant\": %s,\n", ns->constant ? "true" : "false");
  // Export level property
  fwrite(indent, 1, (indent_level + 1)*indent_size, file);
  fprintf(file, "\"level\": %d,\n", ns->level);
  // Export labels
  fwrite(indent, 1, (indent_level + 1)*indent_size, file);
  fprintf(file, "\"labels\": [\n");
  for (size_t i = 0; i < hh_darray_get_item_fill(&ns->labels); i++) {
    label_t label;
    hh_darray_get(&ns->labels, i, &label);
    fwrite(indent, 1, (indent_level + 2)*indent_size, file);
    fprintf(file, "{\n");
    fwrite(indent, 1, (indent_level + 3)*indent_size, file);
    fprintf(file, "\"name\": \"%s\",\n", label.name.text);
    fwrite(indent, 1, (indent_level + 3)*indent_size, file);
    fprintf(file, "\"is_vector\": %s", label.is_vector ? "true" : "false");
    if(label.is_evaluated){
      fprintf(file, ",\n");
      fwrite(indent, 1, (indent_level + 3)*indent_size, file);
      fprintf(file, "\"value\": %ld", label.value);
    }
    fprintf(file, "\n");
    fwrite(indent, 1, (indent_level + 2)*indent_size, file);
    fprintf(file, "}%s\n", i < hh_darray_get_item_fill(&ns->labels) - 1 ? "," : "");
  }
  fwrite(indent, 1, (indent_level + 1)*indent_size, file);
  fprintf(file, "],\n");
  // Export child namespaces
  fwrite(indent, 1, (indent_level + 1)*indent_size, file);
  fprintf(file, "\"children\": [\n");
  for (size_t i = 0; i < hh_darray_get_item_fill(&ns->childs); i++) {
    namespace_t* child = hh_darray_get_reference(&ns->childs, i);
    lasm_export_json_namespace(child, file, indent_level + 2);
    fprintf(file, "%s\n", i < hh_darray_get_item_fill(&ns->childs) - 1 ? "," : "");
  }
  fwrite(indent, 1, (indent_level + 1)*indent_size, file);
  fprintf(file, "]\n");
  // Closing brace
  fwrite(indent, 1, indent_level*indent_size, file);
  fprintf(file, "}");
}
//-----------------------------------------------------------------------------
uint8_t lasm_namespace_deinit(namespace_t *namespace){
  for(size_t i = 0; i < hh_darray_get_item_fill(&namespace->labels); i++){
    label_t label;
    hh_darray_get(&namespace->labels, i, &label);
    lasm_label_deinit(&label);
  }
  hh_darray_deinit(&namespace->labels);
  for(size_t i = 0; i < hh_darray_get_item_fill(&namespace->childs); i++){
    namespace_t* child = hh_darray_get_reference(&namespace->childs, i);
    lasm_namespace_deinit(child);
  }
  hh_darray_deinit(&namespace->childs);
  return 0;
}
//-----------------------------------------------------------------------------
uint8_t lasm_label_deinit(label_t *label){
  parser_expression_deinit(&label->expression);
  return 0;
}
