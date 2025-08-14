#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define HH_DARRAY_IMPLEMENTATION
#include "hh_darray.h"
#define LASM_TOKENIZER_IMPLEMENTATION
#include "lasm_tokenizer.h"
#define LASM_MACRO_IMPLEMENTATION
#include "lasm_macro.h"
#define HH_BIGINT_IMPLEMENTATION
#include "hh_bigint.h"
#include "lasm_namespace.h"
#include "lasm_assembler.h"
#include "lasm_parser.h"
#include "cpu/6502.c"

//-----------------------------------------------------------------------------
uint8_t parse_arguments(int argc, char *argv[]);
uint8_t get_arg_index(int argc, char *argv[], const char word[]);
char* extract_folder_path(const char* path);
hh_darray_t tokens;
hh_darray_t byte_out;
hh_darray_t include_paths;
char output_name[MAX_TOKEN_SIZE];
FILE *input_file;

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]){
  // Parse Arguments
  if(parse_arguments(argc, argv) == ERR) return 0;
  
  //====================================
  // Tokenize input file
  hh_darray_init(&tokens, sizeof(token_t));
  if(lasm_tokenize(input_file, argv[1], &tokens) == ERR) return 0;
  // Find and apply includes
  if(lasm_find_apply_includes(&tokens, &include_paths) == ERR) return 0;
  // Extract macros
  hh_darray_t macros; hh_darray_init(&macros, sizeof(hh_darray_t));
  if(lasm_extract_macros(&tokens, &macros) == ERR) return ERR;
  // Apply macros main tokens
  if(lasm_apply_macros(&tokens, &macros) == ERR) return ERR;
  if(lasm_newline_after_branches(&tokens) == ERR) return ERR;
  if(lasm_clear_multi_newlines(&tokens) == ERR) return ERR;
  //====================================
  // Parse selected cpu and assemble tokens
  lasm_namespace_init(&tokens);

  //====================================
  uint8_t cpu_i = get_arg_index(argc, argv, "-m"); // Machine cpu name
  if(cpu_i){
    if(strcmp(argv[cpu_i+1], "6502") == 0){
      printf("Assembling for 6502...\n");
      lasm_6502_init();
      lasm_assembler.machine_assemble = lasm_6502_assemble;
    }else{
      printf("[ERROR] Machine named '%s' not found\n", argv[cpu_i+1]);
      return 0;
    }
  }else{
    printf("[ERROR] No machine specified\n");
    return 0;
  }
  //print_tokens_as_code(&tokens);

  FILE *json_f = fopen("global_space.json", "w");
  lasm_namespace_to_json(&global_space, json_f, 0);
  fclose(json_f);

  // Assemble tokens
  FILE *output = fopen(output_name, "w+");
  if(lasm_assemble(&tokens, output) == ERR){
    printf("[ERROR] Assembling failed\n");
    //print_tokens_as_code(&tokens);
    fclose(output);
    return 0;
  }
  fclose(output);


  json_f = fopen("global_space.json", "w");
  lasm_namespace_to_json(&global_space, json_f, 0);
  fclose(json_f);
  
  //...
  printf("Done!\n");
  hh_darray_deinit(&tokens);
  hh_darray_deinit(&macros);
  hh_darray_deinit(&include_paths);
  return 0;
}

//-----------------------------------------------------------------------------
uint8_t parse_arguments(int argc, char *argv[]){
  if(argc < 2){
    printf("[ERROR] no input file\n");
      return 0;
  }
  // Parse and extract include paths
  hh_darray_init(&include_paths, sizeof(size_t));
  input_file = fopen(argv[1], "r");
  if(input_file == NULL){
    printf("[ERROR] No file found named '%s'\n", argv[1]);
    return 0;
  }
  char *path = extract_folder_path(argv[1]);
  hh_darray_append(&include_paths, &path);
  // Parse output file name if there is any
  uint8_t out_i = get_arg_index(argc, argv, "-o"); // Output file name
  memset(output_name, 0, sizeof(output_name));
  if(out_i) strcat(output_name, argv[out_i+1]);
  else strcat(output_name, "a.out");
  return 0;
}

//-----------------------------------------------------------------------------
uint8_t get_arg_index(int argc, char *argv[], const char word[]){
  for(uint8_t i = 0; i < argc-1; i++){
    if(strcmp(argv[i], word) == 0){
      return(i);
    }
  }
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
