//-----------------------------------------------------------------------------
// Another command line argument parser header-only module.
// Use "#define HH_ARGPARSE_IMPLEMENTATION" ones in your C file to
// implement the functions of the module
//-----------------------------------------------------------------------------
// Author       : github.com/SMDHuman
// Last Update  : 27.08.2025
//-----------------------------------------------------------------------------
#ifndef HH_ARGPARSE_H
#define HH_ARGPARSE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//-----------------------------------------------------------------------------
typedef struct{
  int argc;
  char **argv;
}hh_argparse_t;

hh_argparse_t* hh_argparse_init(int argc, char *argv[]);
void hh_argparse_deinit(hh_argparse_t *parser);
char* hh_argparse_get_op_short(hh_argparse_t *parser, const char short_op);
char* hh_argparse_get_op_long(hh_argparse_t *parser, const char *long_op);
char* hh_argparse_get_positional(hh_argparse_t *parser, const int index);

//-----------------------------------------------------------------------------
#ifdef HH_ARGPARSE_IMPLEMENTATION
hh_argparse_t* hh_argparse_init(int argc, char *argv[]){
  hh_argparse_t *parser = malloc(sizeof(hh_argparse_t));
  parser->argc = argc;
  parser->argv = (char**)malloc(sizeof(char*) * argc);
  for(int i = 0; i < argc; i++){
    parser->argv[i] = strdup(argv[i]);
  }
  return parser;
}

void hh_argparse_deinit(hh_argparse_t *parser){
  for(int i = 0; i < parser->argc; i++){
    free(parser->argv[i]);
  }
  free(parser->argv);
  free(parser);
}

char* hh_argparse_get_op_short(hh_argparse_t *parser, const char short_op){
  for(int i = 0; i < parser->argc; i++){
    if(parser->argv[i][0] == '-' && parser->argv[i][1] == short_op){
      return parser->argv[i+1];
    }
  }
  return NULL;
}

char* hh_argparse_get_op_long(hh_argparse_t *parser, const char *long_op){
  for(int i = 0; i < parser->argc; i++){
    if(strcmp(parser->argv[i], long_op) == 0){
      return parser->argv[i+1];
    }
  }
  return NULL;
}

char* hh_argparse_get_positional(hh_argparse_t *parser, const int index){
  int count = 0;
  for(int i = 1; i < parser->argc; i++){
    if(parser->argv[i][0] != '-'){
      if(count == index){
        return parser->argv[i];
      }
      count++;
    }else{
      i++;
    }
  }
  return NULL;
}

#endif // HH_ARGPARSE_IMPLEMENTATION
#endif // HH_ARGPARSE_H