#include "fake6502.c"
#define HH_ARGPARSE_IMPLEMENTATION
#include "hh_argparse.h"

int main(int argc, char *argv[]){
  hh_argparse_t* argparse = hh_argparse_init(argc, argv);
  if(argc == 1 || hh_argparse_check_op_short_or_long(argparse, 'h', "help")) {
    printf("Usage: %s [options] <input_file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h, --help       Show this help message\n");
    return 0;
  }
}

uint8_t read6502(uint16_t address) {
  // Read a byte from the specified address
  return 0;
}

void write6502(uint16_t address, uint8_t value) {
  // Write a byte to the specified address
}