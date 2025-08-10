//-----------------------------------------------------------------------------
// lasm_parser.c 10.08.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "lasm_parser.h"
#include "lasm_tokenizer.h"

//-----------------------------------------------------------------------------
// Function Implementations
//-----------------------------------------------------------------------------

namespace_t global_space; // Global namespace for labels

uint8_t lasm_parser_init(hh_darray_t *tokens){
  // Initialize the global namespace
  global_space = (namespace_t){0};
  global_space.constant = 1;
  hh_darray_init(&global_space.labels, sizeof(label_t));
  hh_darray_init(&global_space.namespaces, sizeof(namespace_t));
  // Extract labels from tokens
  int32_t level = 0;
  namespace_t *current_namespace = &global_space;
  hh_darray_t namespace_stack;
  hh_darray_init(&namespace_stack, sizeof(void*));
  for(uint32_t i = 0; i < hh_darray_get_item_fill(tokens); i++){
    token_t *token = hh_darray_get_reference(tokens, i);
    if(token->id == WORD){
      i++;
      token_t *next_token = hh_darray_get_reference(tokens, i);
      if(next_token->id == CBRAC_O){
        level++;
        namespace_t namespace = {0};
        namespace.level = level;
        namespace.constant = 1;
        memcpy(&namespace.name, token, sizeof(token_t));
        //...
        hh_darray_init(&namespace.labels, sizeof(label_t));
        hh_darray_init(&namespace.namespaces, sizeof(namespace_t));
        //...
        hh_darray_append(&current_namespace->namespaces, &namespace);
        hh_darray_append(&namespace_stack, &current_namespace);
        //...
        current_namespace = hh_darray_get_end_reference(&current_namespace->namespaces);
      }else{
        i--;
      }
    }
    else if(token->id == CBRAC_O){
      level++;
      namespace_t namespace = {0};
      namespace.level = level;
      namespace.constant = 0;
      //...
      memcpy(&namespace.name, token, sizeof(token_t));
      memset(&namespace.name.text, 0, sizeof(namespace.name.text));
      //...
      hh_darray_init(&namespace.labels, sizeof(label_t));
      hh_darray_init(&namespace.namespaces, sizeof(namespace_t));
      //...
      hh_darray_append(&current_namespace->namespaces, &namespace);
      hh_darray_append(&namespace_stack, &current_namespace);
      //...
      current_namespace = hh_darray_get_end_reference(&current_namespace->namespaces);
    }
    else if(token->id == CBRAC_C){
      if(level != 0) {
        level--;
        hh_darray_popend(&namespace_stack, &current_namespace);
      } else {
        print_error_loc(token);
        printf("Unmatched closing brace\n");
        return ERR;
      }
    }
    else if(token->id == COLON){
      token_t *prev_token = hh_darray_get_reference(tokens, i - 1);
      if(prev_token->id == WORD){
        label_t label = {0};
        memcpy(&label.name, prev_token, sizeof(token_t));
        label.vector = 0; // Default value
        hh_darray_init(&label.address_expression, sizeof(token_t));
        // Add the label to the current namespace
        hh_darray_append(&current_namespace->labels, &label);
      }
      else if(prev_token->id == SBRAC_C){
        label_t label = {0};
        label.vector = 0; // Default value
        hh_darray_init(&label.address_expression, sizeof(token_t));
        int32_t j = -2;
        token_t *srch_token =  hh_darray_get_reference(tokens, i + j);
        while(srch_token->id != SBRAC_O){
          hh_darray_push(&label.address_expression, 0, srch_token);
          j--;
          if(i + j < 0) {
            print_error_loc(prev_token);
            printf("Unmatched closing bracket\n");
            return ERR;
          } // Prevent out of bounds
          srch_token = hh_darray_get_reference(tokens, i + j);
        }
        j--;
        srch_token = hh_darray_get_reference(tokens, i + j);
        if(srch_token->id != WORD){
          print_error_loc(srch_token);
          printf("Expected word token but found '%s'\n", srch_token->text);
          return ERR;
        }
        memcpy(&label.name, srch_token, sizeof(token_t));
        hh_darray_append(&current_namespace->labels, &label);
      }
    }
  }
  //------------------------------------------------------------
  return 0;
}
// Helper function to print indentation
static void print_indent(int level, FILE *file){
  for (int i = 0; i < level; i++) {
    fprintf(file, "  ");
  }
}
// Convert namespace to JSON recersively 
void lasm_parser_namespace_to_json(namespace_t *namespace, FILE *file, int indent_level) {
  // Opening brace
  print_indent(indent_level, file);
  fprintf(file, "{\n");
  // Name, level, constant fields
  print_indent(indent_level + 1, file);
  fprintf(file, "\"name\": \"%s\",\n", namespace->name.text);
  print_indent(indent_level + 1, file);
  fprintf(file, "\"level\": %d,\n", namespace->level);
  print_indent(indent_level + 1, file);
  fprintf(file, "\"constant\": %d,\n", namespace->constant);  
  // Labels array
  print_indent(indent_level + 1, file);
  fprintf(file, "\"labels\": {\n");
  for (size_t i = 0; i < hh_darray_get_item_fill(&namespace->labels); i++) {
    label_t *label = hh_darray_get_reference(&namespace->labels, i);
    print_indent(indent_level + 2, file);
    fprintf(file, "\"%s\": [", label->name.text);
    // Print the label's address expression
    for (size_t j = 0; j < hh_darray_get_item_fill(&label->address_expression); j++) {
      token_t *token = hh_darray_get_reference(&label->address_expression, j);
      fprintf(file, "\"%s\"", token->text);
      if (j < hh_darray_get_item_fill(&label->address_expression) - 1) {
        fprintf(file, ", ");
      }
    }
    fprintf(file, "]%s\n", (i < hh_darray_get_item_fill(&namespace->labels) - 1) ? "," : "");
  }
  print_indent(indent_level + 1, file);
  fprintf(file, "},\n");
  
  // Namespaces array
  print_indent(indent_level + 1, file);
  fprintf(file, "\"namespaces\": [\n");
  for (size_t i = 0; i < hh_darray_get_item_fill(&namespace->namespaces); i++) {
    namespace_t *child_namespace = hh_darray_get_reference(&namespace->namespaces, i);
    lasm_parser_namespace_to_json(child_namespace, file, indent_level + 2);
    
    // Add comma if not the last element
    if (i < hh_darray_get_item_fill(&namespace->namespaces) - 1) {
      print_indent(indent_level + 2, file);
      fprintf(file, ",\n");
    }
  }
  fprintf(file, "\n");
  print_indent(indent_level + 1, file);
  fprintf(file, "]\n");
  
  // Closing brace
  print_indent(indent_level, file);
  fprintf(file, "}");
}

void lasm_print_namespace(namespace_t *namespace){
  printf("Namespace: %s\n", namespace->name.text);
  printf("  Level: %d\n", namespace->level);
  printf("  Constant: %d\n", namespace->constant);
  printf("  Labels:\n");
  for (size_t i = 0; i < hh_darray_get_item_fill(&namespace->labels); i++) {
    label_t *label = hh_darray_get_reference(&namespace->labels, i);
    printf("    - %s\n", label->name.text);
  }
  printf("  Child Namespaces:\n");
  for (size_t i = 0; i < hh_darray_get_item_fill(&namespace->namespaces); i++) {
    namespace_t *child_namespace = hh_darray_get_reference(&namespace->namespaces, i);
    printf("    - %s\n", child_namespace->name.text);
  }
}