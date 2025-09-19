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

#include "hardware/hardware.h"

char* extract_folder_path(const char* path);

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]){
  // Parse Arguments
  hh_argparse_t *parser = hh_argparse_init(argc, argv);
  if(parser == NULL){
    printf("[ERROR] Argument parsing failed\n");
    return ERR;
  }
  if(hh_argparse_check_op_short(parser, 'h') || hh_argparse_check_op_long(parser, "help") || argc == 1){
    printf("Usage: %s [options] <input_file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h, --help       Show this help message\n");
    printf("  -o, --output     Specify output file\n");
    printf("  -m, --machine    Specify target machine\n");
    printf("  -i, --include     Specify include path\n");
    printf("Machines:\n");
    printf("  6502\n");
    hh_argparse_deinit(parser);
    return 0;
  }
  //====================================
  hh_darray_t include_paths; hh_darray_init(&include_paths, sizeof(char*));
  char *input_file_path = hh_argparse_get_positional(parser, 0);
  FILE *input_file = fopen(input_file_path, "r");
  if(input_file == NULL){
    printf("[ERROR] No file found named '%s'\n", input_file_path);
    return ERR;
  }
  //..
  char *path = extract_folder_path(input_file_path);
  hh_darray_append(&include_paths, &path);
  for(uint8_t i = 0; i < hh_argparse_check_op_short_or_long(parser, 'i', "include"); i++){
    path = hh_argparse_get_nth_op_short_or_long(parser, 'i', "include", i);
    hh_darray_append(&include_paths, &path);
  }
  //====================================
  // Tokenize input file
  hh_darray_t tokens; hh_darray_init(&tokens, sizeof(token_t));
  printf("Tokenizing %s\n", input_file_path);
  if(lasm_tokenize(input_file, input_file_path, &tokens) == ERR) return ERR;
  fclose(input_file);
  //====================================
  // Find and apply includes
  printf("Finding and applying includes...\n");
  if(lasm_find_apply_includes(&tokens, &include_paths) == ERR) return ERR;
  hh_darray_t macros; hh_darray_init(&macros, sizeof(macro_t));
  printf("Extracting macros...\n");
  if(lasm_extract_macros(&tokens, &macros) == ERR) return ERR;
  //print_macros(&macros);
  // Apply macros main tokens
  printf("Applying macros...\n");
  if(lasm_apply_macros(&tokens, &macros) == ERR) return ERR;
  if(lasm_newline_after_branches(&tokens) == ERR) return ERR;
  if(lasm_clear_multi_newlines(&tokens) == ERR) return ERR;
  // print_tokens_as_code(&tokens);
  //====================================
  char* cpu = hh_argparse_get_op_short_or_long(parser, 'm', "machine");
  if(cpu){
    uint8_t found = 0;
    for(size_t i = 0; i < sizeof(hardwares)/sizeof(hardware_t); i++){
      if(strcmp(cpu, hardwares[i].name) == 0){
        found = 1;
        lasm_assembler.machine_assemble = (uint8_t (*)(void*))hardwares[i].assembler;
        hardwares[i].initializer(&lasm_assembler);
        printf("Using machine: %s\n", cpu);
        break;
      }
    }
    if(!found){
      printf("[ERROR] Unknown machine: %s\n", cpu);
      return ERR;
    }
  }else{
    printf("[ERROR] No machine specified\n");
    return ERR;
  }
  //====================================
  // Assemble tokens
  char* output_name = hh_argparse_get_op_short_or_long(parser, 'o', "output");
  if(!output_name){
    output_name = "a.out";
  }
  //...
  FILE *output = fopen(output_name, "w+");
  printf("Assembling to %s\n", output_name);
  if(lasm_assemble(&tokens, output) == ERR){
    fclose(output);
    return ERR;
  }
  fclose(output);
  // Export global namespace to JSON
  // FILE *json = fopen("global_namespace.json", "w");
  // lasm_export_json_namespace(&lasm_assembler.global_namespace, json, 0);
  // fclose(json);
  //====================================
  // Free each macro in the macros array
  // TODO:
  hh_darray_deinit(&macros);
  // Free path strings before deinit
  for(size_t i = 0; i < hh_darray_get_item_fill(&include_paths); i++) {
    char *path;
    hh_darray_get(&include_paths, i, &path);
    free(path);
  }
  hh_darray_deinit(&include_paths);
  hh_darray_deinit(&tokens);
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
