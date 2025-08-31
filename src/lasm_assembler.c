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
  lasm_assembler.tokens = tokens;
  lasm_assembler.output_file = output;
  hh_darray_init(&lasm_assembler.backward_patches, sizeof(expression_t));
  //...
  if(lasm_scout_namespace(tokens, 0, &lasm_assembler.global_namespace) == (size_t)-1) return ERR;
  //...
  token_t *token = hh_darray_get_reference(tokens, 0);
  while(hh_darray_get_fill(tokens) > 0){
    uint8_t expression_flag = 0;
    //====================================
    // Handle word token
    if(token->id == WORD){
      // 'word...{'
      if(lasm_is_lineend_id(tokens, 0, CBRAC_O)){
        namespace_t* find = lasm_find_namespace_reachable_namespace(lasm_assembler.current_namespace, token->text);
        // This is already found by scout
        if(find){
          lasm_assembler.current_namespace = find;
          token_t* next_token = hh_darray_get_reference(tokens, 1);
          // 'word[...]{'
          if(next_token->id == SBRAC_O){
            hh_darray_pop(tokens, 0, 0); // Consume name
            hh_darray_pop(tokens, 0, 0); // Consume square brace open
            hh_bigint_t value; hh_bigint_init(&value, 0);
            if(lasm_parse_and_eval_expression(tokens, &value, 0, 0, 0) == ERR) return ERR;
            uint64_t eval_value = hh_bigint_get_uint64(&value);
            fseek(lasm_assembler.output_file, eval_value, SEEK_SET);
            if(lasm_expect_and_skip_id(tokens, SBRAC_C) == ERR) return ERR;
            if(lasm_expect_and_skip_id(tokens, CBRAC_O) == ERR) return ERR;
          }
          // 'word{'
          else if(next_token->id == CBRAC_O){
            hh_darray_pop(tokens, 0, 0); // Consume name
            if(lasm_expect_and_skip_id(tokens, CBRAC_O) == ERR) return ERR;
          }
        }else{
          printf(TAG);
          print_error_loc(token);
          printf("Namespace named '%s' already exists in same namespace at '%s':%d:%d\n", token->text, lasm_assembler.current_namespace->name.filename, lasm_assembler.current_namespace->name.line, lasm_assembler.current_namespace->name.col);
          return ERR;
        }
      }
      // 'word...:'
      else if(lasm_is_lineend_id(tokens, 0, COLON)){
        label_t* find = lasm_find_label_in_namespace(lasm_assembler.current_namespace, token->text);
        if(find){
          // 'word[...]:'
          if(find->is_vector){
            hh_darray_pop(tokens, 0, 0); // Consume name
            hh_darray_pop(tokens, 0, 0); // Consume square brace open
            hh_bigint_t value; hh_bigint_init(&value, 0);
            if(lasm_parse_and_eval_expression(tokens, &value, 0, 0, 0) == ERR) return ERR;
            find->value = hh_bigint_get_uint64(&value);
            find->is_evaluated = 1;
            fseek(lasm_assembler.output_file, find->value, SEEK_SET);
            if(lasm_expect_and_skip_id(tokens, SBRAC_C) == ERR) return ERR;
            if(lasm_expect_and_skip_id(tokens, COLON) == ERR) return ERR;
          }
          // 'word:'
          else{
            hh_darray_pop(tokens, 0, 0); // Consume name
            if(lasm_expect_and_skip_id(tokens, COLON) == ERR) return ERR;
            find->value = lasm_get_file_cursor();
            find->is_evaluated = 1;
          }
        }else{
          printf(TAG);
          print_error_loc(token);
          printf("Label named '%s' already exists in same namespace at '%s':%d:%d\n", token->text, find->name.filename, find->name.line, find->name.col);
          return ERR;
        }
      }
      // 'word ...'
      else{
        label_t* label = lasm_find_label_reachable_namespace(lasm_assembler.current_namespace, token->text);
        if(label){
          hh_bigint_t value; hh_bigint_init(&value, 0);
          if(lasm_parse_and_eval_expression(tokens, &value, 1, 0, 0) == ERR) return ERR;
          fwrite(value.data, 1, value.size, lasm_assembler.output_file);
          hh_bigint_deinit(&value);
        }
        else{
          if(lasm_assembler.machine_assemble() == ERR) return ERR;
        }
      }
    }
    //====================================
    // Handle '{' token
    else if(token->id == CBRAC_O){
      namespace_t temp = {.constant = 0,
                          .level = lasm_assembler.current_namespace->level + 1,
                          .parent = lasm_assembler.current_namespace,
                          .name = *token};
      hh_darray_append(&lasm_assembler.current_namespace->childs, &temp);
      lasm_assembler.current_namespace = hh_darray_get_end_reference(&lasm_assembler.current_namespace->childs);
      hh_darray_init(&lasm_assembler.current_namespace->childs, sizeof(namespace_t));
      hh_darray_init(&lasm_assembler.current_namespace->labels, sizeof(label_t));
      hh_darray_pop(tokens, 0, 0); // Consume curly brace open
      if(lasm_scout_namespace(tokens, 0, lasm_assembler.current_namespace) == (size_t)-1) return ERR;
    }
    //====================================
    // Handle '}' token
    else if(token->id == CBRAC_C){
      // Backwards patches
      lasm_eval_and_backward_patch_expression(1);
      //...
      namespace_t* upper_ns = (namespace_t *)lasm_assembler.current_namespace->parent;
      if(!lasm_assembler.current_namespace->constant){
        lasm_namespace_deinit(lasm_assembler.current_namespace);
        hh_darray_remove_reference(&((namespace_t *)lasm_assembler.current_namespace->parent)->childs, lasm_assembler.current_namespace);
      }
      lasm_assembler.current_namespace = upper_ns;
      hh_darray_pop(tokens, 0, 0); // Consume curly brace close
     
    }
    //====================================
    else if(token->id == NUMBER || token->id == STRING_DB || token->id == RBRAC_O){
      hh_bigint_t value; hh_bigint_init(&value, 0);
      if(lasm_parse_and_eval_expression(tokens, &value, 1, 0, 0) == ERR) return ERR;
      fwrite(value.data, 1, value.size, lasm_assembler.output_file);
      hh_bigint_deinit(&value);
    }
    //====================================
    // Handle "["
    else if(token->id == SBRAC_O){ 
      hh_darray_pop(tokens, 0, 0); // Consume square brace open
      hh_bigint_t value; hh_bigint_init(&value, 0);
      if(lasm_parse_and_eval_expression(tokens, &value, 0, 0, 0) == ERR) return ERR;
      uint64_t eval_val = hh_bigint_get_uint64(&value);
      fseek(lasm_assembler.output_file, eval_val, SEEK_SET);
      if(lasm_expect_and_skip_id(tokens, SBRAC_C) == ERR) return ERR;
      if(lasm_expect_and_skip_id(tokens, COLON) == ERR) return ERR;
    }
    //====================================
    else{
      printf(TAG);
      print_error_loc(token);
      printf("Unexpected token: %s\n", token->text);
      return ERR;
    }
    //==========================================================
    // Evaluate expressions
    if(expression_flag){
      hh_bigint_t value; hh_bigint_init(&value, 0);
      if(lasm_parse_and_eval_expression(tokens, &value, 1, 0, 0) == ERR) return ERR;
      fwrite(value.data, 1, value.size, lasm_assembler.output_file);
      hh_bigint_deinit(&value);
    }
    //==========================================================
    // Expect newline
    if(lasm_expect_and_skip_id(tokens, NEWLINE) == ERR) return ERR;
  }
  // =====================
  // Backwards patches
  lasm_eval_and_backward_patch_expression(0);
  return 0;
}
//-----------------------------------------------------------------------------
uint8_t lasm_eval_and_backward_patch_expression(uint8_t enable_skip){
  size_t skip_i = 0; 
  while(hh_darray_get_item_fill(&lasm_assembler.backward_patches) > skip_i){
    expression_t* patch = hh_darray_get_reference(&lasm_assembler.backward_patches, skip_i);
    hh_bigint_t value; hh_bigint_init(&value, 0);
    uint8_t res = lasm_evaluate_expression_tree(patch->root, &value);
    if(res == ERR) return ERR;
    if(res == 2){
      if(enable_skip){
        skip_i++;
        continue;
      }
      else{
        printf(TAG);
        print_error_loc(&patch->root->token);
        printf("Expression evaluation failed while backwards patching\n");
        return ERR;
      }
    }
    if(patch->is_relative){
      hh_bigint_subtract_int64(&value, patch->offset + patch->size);
    }
    size_t current_offset = ftell(lasm_assembler.output_file);
    fseek(lasm_assembler.output_file, patch->offset, SEEK_SET);
    fwrite(value.data, 1, value.size < patch->size ? value.size : patch->size, lasm_assembler.output_file);
    fseek(lasm_assembler.output_file, current_offset, SEEK_SET);
    hh_bigint_deinit(&value);
    parser_expression_deinit(patch);
    hh_darray_pop(&lasm_assembler.backward_patches, skip_i, 0);
  }
  return 0;
}

//-----------------------------------------------------------------------------
// Parse and evaluate an expression
uint8_t lasm_parse_and_eval_expression(hh_darray_t* tokens, hh_bigint_t* result, uint8_t enable_backward_patch, uint8_t is_relative, size_t max_size){
  expression_t expression;
  expression.is_relative = is_relative;
  if(parser_expression(tokens, &expression) == ERR) return ERR;
  uint8_t res = lasm_evaluate_expression_tree(expression.root, result);
  if(res == ERR) return ERR;
  if(res == 2){
    if(enable_backward_patch){
      hh_darray_append(&lasm_assembler.backward_patches, 0);
      expression_t* patch = hh_darray_get_end_reference(&lasm_assembler.backward_patches);
      *patch = expression;
      if(max_size > 0) patch->size = result->size < max_size ? result->size : max_size;
      else patch->size = result->size;
      patch->offset = lasm_get_file_cursor();
    }else{
      printf(TAG);
      print_error_loc(&expression.root->token);
      printf("Expression evaluation failed. Evaluation don't support backwards patching.\n");
      return ERR;
    }
  }else{
    parser_expression_deinit(&expression);
  }
  if(max_size > 0 && result->size > max_size){
    hh_bigint_resize(result, max_size);
  }
  if(is_relative){
    hh_bigint_subtract_int64(result, result->size + lasm_get_file_cursor());
  }
  return 0;
}

//-----------------------------------------------------------------------------
// Search and log all reachable child labels and namespaces
size_t lasm_scout_namespace(hh_darray_t* tokens, size_t start_from, namespace_t *namespace){
  size_t i = start_from;
  while(i < hh_darray_get_item_fill(tokens)){
    token_t* token = hh_darray_get_reference(tokens, i);
    if(token->id == WORD){
      // 'word...{'
      if(lasm_is_lineend_id(tokens, i, CBRAC_O)){
        namespace_t* find = lasm_find_namespace_reachable_namespace(namespace, token->text);
        if(!find){
          namespace_t temp = {.constant = 1,
                              .level = namespace->level + 1,
                              .parent = namespace,
                              .name = *token};
          hh_darray_append(&namespace->childs, &temp);
          namespace_t* child_ns = hh_darray_get_end_reference(&namespace->childs);
          hh_darray_init(&child_ns->childs, sizeof(namespace_t));
          hh_darray_init(&child_ns->labels, sizeof(label_t));
          token_t *skip_tokens = hh_darray_get_reference(tokens, i);
          while(skip_tokens->id != CBRAC_O){
            i++;
            skip_tokens = hh_darray_get_reference(tokens, i);
          }i++;
          i = lasm_scout_namespace(tokens, i, child_ns);
          if(i == (size_t)-1) return -1;
        }else{
          printf(TAG);
          print_error_loc(token);
          printf("Namespace named '%s' already reachable and exists at '%s':%d:%d\n", token->text, find->name.filename, find->name.line, find->name.col);
          return -1;
        }
      }
      // 'word...:'
      else if(lasm_is_lineend_id(tokens, i, COLON)){
        label_t* find = lasm_find_label_in_namespace(namespace, token->text);
        if(!find){
          hh_darray_append(&namespace->labels, 0);
          label_t* new_label = hh_darray_get_end_reference(&namespace->labels);
          new_label->name = *token;
          // 'word[...]:'
          token_t *next_token = hh_darray_get_reference(tokens, i+1);
          if(next_token->id == SBRAC_O){
            new_label->is_vector = 1;
            new_label->is_evaluated = 0;
            // Skip to colon
            size_t level_count = 1;
            i += 2; // consume name and open bracket
            while(level_count > 0){
              token_t *skip_token = hh_darray_get_reference(tokens, i++);
              if(skip_token->id == SBRAC_O) level_count++;
              else if(skip_token->id == SBRAC_C) level_count--;
            }
          }
          // 'word:'
          else{
            i += 1; //consume name
            new_label->is_vector = 0;
            new_label->is_evaluated = 0;
          }
        }else{
          printf(TAG);
          print_error_loc(token);
          printf("Label named '%s' already exists in same namespace at '%s':%d:%d\n", token->text, find->name.filename, find->name.line, find->name.col);
          return -1;
        }
        // Skip to colon
        token_t *skip_token = hh_darray_get_reference(tokens, i);
        if(skip_token->id != COLON){
          printf(TAG);
          print_error_loc(token);
          printf("Expected ':' but got '%s'\n", skip_token->text);
          return -1;
        }
      }
      else{
        i++;
      }
    }
    else if(token->id == CBRAC_O){
      size_t level_count = 1;
      i++;
      while(level_count > 0){
        token_t *skip_token = hh_darray_get_reference(tokens, i++);
        if(skip_token->id == CBRAC_O) level_count++;
        else if(skip_token->id == CBRAC_C) level_count--;
      }
    }
    else if(token->id == CBRAC_C){
      i++;
      return i;
    }
    else{
      i++;
    }
  }
  return i;
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
    uint8_t left_res = lasm_evaluate_expression_tree((expression_tree_t*)node->left, &left_number); // Evaluate left subtree
    hh_bigint_t right_number; hh_bigint_init(&right_number, 0);
    uint8_t right_res = lasm_evaluate_expression_tree((expression_tree_t*)node->right, &right_number); // Evaluate right subtree
    if(left_res || right_res){
      if(number->size < left_number.size || number->size < right_number.size){
        size_t biggest_size = (left_number.size > right_number.size ? left_number.size : right_number.size);
        if(hh_bigint_resize(number, biggest_size) == ERR) return ERR;
      }
      return left_res | right_res;
    }
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
    uint8_t backslash = 0;
    size_t backslash_count = 0;
    uint8_t chr;
    for(size_t i = 0; ; i++){
      chr = node->token.text[i];
      if(chr == '\0') break;
      if(chr == '\\' && !backslash){
        backslash = 1;
        backslash_count++;
        continue;
      }
      if(backslash){
        switch(chr){
          case 'n': chr = '\n'; break;
          case 'r': chr = '\r'; break;
          case 't': chr = '\t'; break;
          case '\\': chr = '\\'; break;
          case '\'': chr = '\''; break;
          case '\"': chr = '\"'; break;
          case '0': chr = '\0'; break;
          default:
            printf(TAG);
            print_error_loc(&node->token);
            printf("Unknown escape sequence: \\%c\n", chr);
            return ERR;
        }
        backslash = 0;
      }
      hh_bigint_set_at(number, chr, i-backslash_count);
    }
  }else if(node->token.id == WORD){
    label_t* label = lasm_find_label_reachable_namespace(lasm_assembler.current_namespace, node->token.text);
    if(label != NULL){
      // If label is found, use its value
      if(label->is_evaluated){
        hh_bigint_resize(number, lasm_assembler.addressing_size);
        memcpy(number->data, &label->value, lasm_assembler.addressing_size);
        if(label->is_vector){
          hh_bigint_normalize(number);
        }
      }else{
        hh_bigint_resize(number, lasm_assembler.addressing_size);
        hh_bigint_set_zero(number);
        return 2;
      }
    }else{
      printf(TAG);
      print_error_loc(&node->token);
      printf("Unknown token '%s'\n", node->token.text);
      return ERR;
    }
  }
  return 0;
}
//-----------------------------------------------------------------------------
label_t* lasm_find_label_in_namespace(namespace_t* namespace, const char* name){
  for(size_t i = 0; i < hh_darray_get_item_fill(&namespace->labels); i++){
    label_t *label = hh_darray_get_reference(&namespace->labels, i);
    if(strcmp(label->name.text, name) == 0) return label;
  }
  return NULL;
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
namespace_t* lasm_find_namespace_reachable_namespace(namespace_t* namespace, const char* name){
  if(strcmp(namespace->name.text, name) == 0) return namespace;
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
        namespace_t *lower_ns = lasm_find_namespace_reachable_namespace(ns, sub);
        if(lower_ns) return lower_ns;
      }
    }
  }
  for(size_t i = 0; i < hh_darray_get_item_fill(&namespace->childs); i++){
    namespace_t *ns = hh_darray_get_reference(&namespace->childs, i);
    if(strcmp(ns->name.text, name) == 0){
      return ns;
    }
  }
  if(namespace->parent != NULL){
    return lasm_find_namespace_reachable_namespace(namespace->parent, name);
  }
  return NULL;
}

//-----------------------------------------------------------------------------
void lasm_export_json_namespace(namespace_t* ns, FILE* file, uint8_t indent_level){
  static char indent[256]; memset(indent, ' ', 256);
  const uint8_t indent_size = 2;
  fwrite(indent, 1, indent_level*indent_size, file);
  fprintf(file, "{\n");
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
    label_t* label = hh_darray_get_reference(&namespace->labels, i);
    lasm_label_deinit(label);
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
