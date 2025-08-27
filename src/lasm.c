#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define HH_DARRAY_IMPLEMENTATION
#include "hh_darray.h"
#define HH_BIGINT_IMPLEMENTATION
#define INITIAL_CAPACITY 1
#include "hh_bigint.h"
#define HH_ARGPARSE_IMPLEMENTATION
#include "hh_argparse.h"

#include "lasm_tokenizer.h"
#include "lasm_macro.h"
#include "lasm_assembler.h"
#include "lasm_parser.h"

#include "cpu/6502.c"

//-----------------------------------------------------------------------------
char* extract_folder_path(const char* path);
hh_darray_t tokens;

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]){
  // Parse Arguments
  hh_argparse_t *parser = hh_argparse_init(argc, argv);
  if(parser == NULL){
    printf("[ERROR] Argument parsing failed\n");
    return 1;
  }
  hh_darray_t include_paths; hh_darray_init(&include_paths, sizeof(char*));
  char *input_file_path = hh_argparse_get_positional(parser, 0);
  FILE *input_file = fopen(input_file_path, "r");
  if(input_file == NULL){
    printf("[ERROR] No file found named '%s'\n", input_file_path);
    return 0;
  }
  
  //====================================
  // Tokenize input file
  hh_darray_init(&tokens, sizeof(token_t));
  printf("Tokenizing %s\n", input_file_path);
  if(lasm_tokenize(input_file, input_file_path, &tokens) == ERR) return 0;
  fclose(input_file);
  // Find and apply includes
  printf("Finding and applying includes...\n");
  if(lasm_find_apply_includes(&tokens, &include_paths) == ERR) return 0;
  // Extract macros
  hh_darray_t macros; hh_darray_init(&macros, sizeof(hh_darray_t));
  printf("Extracting macros...\n");
  if(lasm_extract_macros(&tokens, &macros) == ERR) return ERR;
  printf("Extracted %zu macros\n", hh_darray_get_item_fill(&macros));
  // Apply macros main tokens
  if(lasm_apply_macros(&tokens, &macros) == ERR) return ERR;
  if(lasm_newline_after_branches(&tokens) == ERR) return ERR;
  if(lasm_clear_multi_newlines(&tokens) == ERR) return ERR;
  //print_tokens_as_code(&tokens);
  //====================================
  // Parse selected cpu and assemble tokens
  char* cpu = hh_argparse_get_op_short(parser, 'm');
  if(cpu){
    if(strcmp(cpu, "6502") == 0){
      printf("Assembling for 6502...\n");
      lasm_6502_init();
      lasm_assembler.machine_assemble = lasm_6502_assemble;
    }else{
      printf("[ERROR] Machine named '%s' not found\n", cpu);
      return 0;
    }
  }else{
    printf("[ERROR] No machine specified\n");
    return 0;
  }
  //====================================

  // Assemble tokens
  char* output_name = hh_argparse_get_op_short(parser, 'o');
  if(!output_name){
    output_name = "a.out";
  }
  FILE *output = fopen(output_name, "w+");
  printf("Assembling to %s\n", output_name);
  if(lasm_assemble(&tokens, output) == ERR){
    printf("[ERROR] Assembling failed\n");
    //print_tokens_as_code(&tokens);
    fclose(output);
    return 0;
  }
  fclose(output);

  //...
  FILE *json = fopen("build/global_namespace.json", "w");
  lasm_export_json_namespace(&lasm_assembler.global_namespace, json, 0);
  fclose(json);

  //...
  hh_darray_deinit(&tokens);
  
  // Free each macro in the macros array
  for(size_t i = 0; i < hh_darray_get_item_fill(&macros); i++) {
    hh_darray_t macro;
    hh_darray_get(&macros, i, &macro);
    hh_darray_deinit(&macro);
    // Free the allocated macro_tokens if any
  }
  hh_darray_deinit(&macros);
  
  // Free path strings before deinit
  for(size_t i = 0; i < hh_darray_get_item_fill(&include_paths); i++) {
    char *path;
    hh_darray_get(&include_paths, i, &path);
    free(path);
  }
  hh_darray_deinit(&include_paths);
  
  lasm_namespace_deinit(&lasm_assembler.global_namespace);
  hh_darray_deinit(&lasm_assembler.backward_patches);
  hh_argparse_deinit(parser);
  printf("Done!\n");
  return 0;
}

//-----------------------------------------------------------------------------
char* extract_folder_path(const char* path){
  uint16_t size = 0;
  for(uint16_t i = 0; path[i] != 0; i++){
    if(path[i] == '/' || path[i] == '\\') size = i;
  }
  char* folder_path = malloc(size+1);
  memcpy(folder_path, path, size);
  folder_path[size] = 0;
  return(folder_path);
}
