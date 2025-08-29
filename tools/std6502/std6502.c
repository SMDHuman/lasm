//-----------------------------------------------------------------------------
// This is a simple 6502 commandline emulator that implements some of 
// C standard library functions as syscalls. 
//-----------------------------------------------------------------------------
// std6502.c
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include "fake6502.c"
#define HH_ARGPARSE_IMPLEMENTATION
#include "hh_argparse.h"

uint8_t memory[65536] = {0};

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
  while(1){
    step6502();
    printf("A: %01X\n", a);
    printf("PC: %02X\n", pc);
    printf("instructions: %x\n", opcode);
    if (pc == 0) break; // Simple halt condition
  }

  hh_argparse_deinit(argparse);
  return 0;
}

//-----------------------------------------------------------------------------
uint8_t read6502(uint16_t address) {
  // Read a byte from the specified address
  printf("Reading from address %04X: %02X\n", address, memory[address]);
  return memory[address];
}

void write6502(uint16_t address, uint8_t value) {
  // Write a byte to the specified address
  printf("Writing to address %04X: %02X\n", address, value);
  memory[address] = value;
}