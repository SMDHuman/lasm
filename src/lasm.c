#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define HH_DARRAY_IMPLEMENTATION
#include "../include/hh_darray.h"
#define LASM_TOKENIZER_IMPLEMENTATION
#include "../include/lasm_tokenizer.h"
#define LASM_MACRO_IMPLEMENTATION
#include "../include/lasm_macro.h"
#include "../include/lasm_parser.h"
#include "cpu/6502.c"

//-----------------------------------------------------------------------------
uint8_t get_arg_index(int argc, char *argv[], const char word[]);
char* extract_folder_path(const char* path);
hh_darray_t tokens;
hh_darray_t byte_out;

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]){
  // Parse Arguments
  if(argc < 2){
    printf("[ERROR] no input file\n");
      return 0;
  }
  hh_darray_t include_paths;
  hh_darray_init(&include_paths, sizeof(size_t));
  FILE *file = fopen(argv[1], "r");
  if(file == NULL){
    printf("[ERROR] No file found named '%s'\n", argv[1]);
    return 0;
  }
  char *path = extract_folder_path(argv[1]);
  hh_darray_append(&include_paths, &path);
  // Parse output file name if there is any
  uint8_t out_i = get_arg_index(argc, argv, "-o"); // Output file name
  char output_name[MAX_TOKEN_SIZE] = {0};
  if(out_i) strcat(output_name, argv[out_i+1]);
  else strcat(output_name, "a.out");
  //uint8_t cpu_i = get_arg_index(argc, argv, "-m"); // Machine cpu name

  //====================================
  // Tokenize input file
  hh_darray_init(&tokens, sizeof(token_t));
  if(lasm_tokenize(file, argv[1], &tokens) == ERR) return 0;
  // Find and apply includes
  if(lasm_find_apply_includes(&tokens, &include_paths) == ERR) return 0;
  // Extract macros
  hh_darray_t macros; hh_darray_init(&macros, sizeof(hh_darray_t));
  if(lasm_extract_macros(&tokens, &macros) == ERR) return ERR;
  // Apply macros main tokens
  if(lasm_apply_macros(&tokens, &macros) == ERR) return ERR;
  if(lasm_newline_after_branches(&tokens) == ERR) return ERR;
  if(lasm_clear_multi_newlines(&tokens) == ERR) return ERR;
  //print_tokens_as_code(&tokens);
  //====================================
  // Parse selected cpu and assemble tokens

  lasm_parser_init(&tokens);

  FILE *outf = fopen("global_space.json", "w");
  lasm_parser_namespace_to_json(&global_space, outf, 0);
  fclose(outf);
  
  //====================================
  /*FILE *outf = fopen(output_name, "w");
  if(cpu_i){
    if(strcmp(argv[cpu_i+1], "6502") == 0){
      printf("Assembling for 6502...\n");
      assemble_6502(&tokens, outf);
    }else{
      printf("[ERROR] Machine named '%s' not found\n", argv[cpu_i+1]);
      return 0;
    }
  }else{
    printf("[ERROR] No machine specified\n");
    return 0;
  }
  fclose(outf);
  */
  //...
  printf("Done!\n");
  hh_darray_deinit(&tokens);
  hh_darray_deinit(&macros);
  hh_darray_deinit(&include_paths);
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
