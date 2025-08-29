//-----------------------------------------------------------------------------
// This is a simple 6502 commandline emulator that implements some of 
// C standard library functions as syscalls. 
// To call a function, you use 0x1100 address. 
// Store to that address argument size you provide 
// after the store instruction, put your instruction id and arguments
// Return value is stored in 0x11101 - 0x11ff with 255 bytes
//-----------------------------------------------------------------------------
// std6502.c
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include "fake6502.c"
#define HH_ARGPARSE_IMPLEMENTATION
#include "hh_argparse.h"

/*
    void return (int32_t exit_code)      : opcode[2], exit_code[4]    : 0x0000 
    int32_t puts (uint16_t str_pointer)  : opcode[2], str_pointer[2]  : 0x0001 
*/
void print_all_status();

uint8_t memory[65536] = {0};
uint8_t running = 1;
int32_t exit_code = 0;

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]){
  hh_argparse_t* argparse = hh_argparse_init(argc, argv);
  if(argc == 1 || hh_argparse_check_op_short_or_long(argparse, 'h', "help")) {
    printf("Usage: %s [options] <input_file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h, --help       Show this help message\n");
    return 0;
  }
  // Load the input file into memory
  const char *input_file = hh_argparse_get_positional(argparse, 0);
  if(!input_file) {
    fprintf(stderr, "Error: No input file specified\n");
    return 1;
  }
  FILE *file = fopen(input_file, "rb");
  if(!file) {
    fprintf(stderr, "Error: Could not open input file '%s'\n", input_file);
    return 1;
  }
  // Load the input file into memory
  fread(memory, 1, sizeof(memory), file);
  fclose(file);

  // Run the 6502 emulator
  reset6502();
  while(running){
    step6502();
    //print_all_status();
    // getchar(); // Wait for user input to proceed to the next step
  }

  hh_argparse_deinit(argparse);
  return exit_code;
}

void print_all_status(){
  printf("A: %d\n", a);
  printf("X: %d\n", x);
  printf("Y: %d\n", y);
  printf("PC: %x\n", pc);
  printf("SP: %x\n", sp);
  printf("============================\n");
}

//-----------------------------------------------------------------------------
void exec_std_functions(){
  uint16_t function_call_value = (memory[pc+4] << 8) | memory[pc+3];
  uint8_t* args = &memory[pc + 5];
  uint8_t argc = memory[0x1100]-2;
  // printf("Function call: %x\n", function_call_value);
  // printf("Arguments: ");
  // for(int i = 0; i < argc; i++) {
  //   printf("%02x ", args[i]);
  // }
  // printf("\n");
  switch(function_call_value){
    case 0x0000: // return
      running = 0;
      exit_code = *(int32_t*)args;
      break;
    case 0x0001: // puts
      int* result = (int*)&memory[0x1110];
      *result = puts((const char*)&memory[*(uint16_t*)args]);
      break;
    // Add more cases for other functions here
  }
}

//-----------------------------------------------------------------------------
uint8_t read6502(uint16_t address) {
  // Read a byte from the specified address
  return memory[address];
}

void write6502(uint16_t address, uint8_t value) {
  memory[address] = value;
  if(address == 0x1100) exec_std_functions();
}