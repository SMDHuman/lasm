#include <stdio.h>
#include <stdint.h>
#include "raylib.h"

#include "fake6502.c"
#define HH_ARGPARSE_IMPLEMENTATION
#include "hh_argparse.h"

#define CRT_BUFFER_ADDRESS 0x1000
#define CRT_WIDTH 80
#define CRT_HEIGHT 60
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

uint8_t memory[65536] = {0};

int main(int argc, char *argv[]) {
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
  // Open the input file and load into memory
  FILE *file = fopen(input_file, "rb");
  if(!file) {
    fprintf(stderr, "Error: Could not open input file '%s'\n", input_file);
    return 1;
  }
  fread(memory, 1, sizeof(memory), file);
  fclose(file);

  // Run the 6502 emulator
  InitWindow(WIN_WIDTH, WIN_HEIGHT, "CRT 6502 Emulator");
  reset6502();
  double last_draw = GetTime();
  uint8_t cpu_running = 1;
  while(!WindowShouldClose()){
    // Halt if PC reaches 0xffff
    if(pc == 0xffff && cpu_running){
      cpu_running = 0;
      printf("CPU Halted\n");
    }
    // Execute the next instruction
    if(cpu_running){
      step6502();
    }
    // Drawing
    if(GetTime() - last_draw > 1.0 / 60.0) {
      BeginDrawing();
      for(int y = 0; y < CRT_HEIGHT; y++) {
        for(int x = 0; x < CRT_WIDTH; x++) {
          uint8_t color = read6502(CRT_BUFFER_ADDRESS + y * CRT_WIDTH + x);
          uint8_t red = (color & 0xE0);
          uint8_t green = (color & 0x1C) << 3;
          uint8_t blue = (color & 0x03) << 6;
          float pixel_size_x = (float)WIN_WIDTH / CRT_WIDTH;
          float pixel_size_y = (float)WIN_HEIGHT / CRT_HEIGHT;
          float pixel_size = pixel_size_y < pixel_size_x ? pixel_size_y : pixel_size_x;
          DrawRectangle(x * pixel_size, y * pixel_size, pixel_size, pixel_size, (Color){red, green, blue, 255});
        }
      }
      EndDrawing();
      last_draw = GetTime();
    }
  }

  // Clean up
  CloseWindow();
  hh_argparse_deinit(argparse);  
  return 0;
}

//-----------------------------------------------------------------------------
uint8_t read6502(uint16_t address) {

  return memory[address];
}
void write6502(uint16_t address, uint8_t value) {
  memory[address] = value;
}