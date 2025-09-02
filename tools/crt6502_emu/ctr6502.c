#include <stdio.h>
#include <stdint.h>
#include "raylib.h"

#include "fake6502.c"
#define HH_ARGPARSE_IMPLEMENTATION
#include "hh_argparse.h"

enum{
  COLOR_RGB332
};

#define HALT_ADDRESS 0xFFFF
#define CRT_WIDTH_ADDRESS 0x1000 - 4
#define CRT_HEIGHT_ADDRESS 0x1000 - 3
#define INPUT_KEY_ADDRESS 0x1000 - 2
#define CRT_BUFFER_ADDRESS 0x1000
#define LOG_ON_WRITE_ADDRESS 0xFFBC

#define CRT_COLOR_MODE COLOR_RGB332 
#define CRT_WIDTH memory[CRT_WIDTH_ADDRESS]
#define CRT_HEIGHT memory[CRT_HEIGHT_ADDRESS]
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

uint8_t memory[65536] = {0};

Color color_converter(uint8_t memcolor);

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
  SetTraceLogLevel(LOG_ERROR); 
  InitWindow(WIN_WIDTH, WIN_HEIGHT, "CRT 6502 Emulator");
  reset6502();
  double last_draw = GetTime();
  uint8_t cpu_running = 1;
  while(!WindowShouldClose()){
    // Halt if PC reaches HALT_ADDRESS
    if(pc == HALT_ADDRESS && cpu_running){
      cpu_running = 0;
      printf("CPU Halted\n");
    }
    // Execute the next instruction
    if(cpu_running){
      step6502();
      // print first 32 byte of memory
      // for(int i = 0; i < 18; i++) {
      //   printf("0x%02X, ", read6502(i));
      // }
      // printf("\n");
    }
    // Drawing and Input handling
    if(GetTime() - last_draw > 1.0 / 60.0) {
      BeginDrawing();
      for(int y = 0; y < CRT_HEIGHT; y++) {
        for(int x = 0; x < CRT_WIDTH; x++) {
          uint8_t color = read6502(CRT_BUFFER_ADDRESS + y * CRT_WIDTH + x);
          float pixel_size_x = (float)WIN_WIDTH / CRT_WIDTH;
          float pixel_size_y = (float)WIN_HEIGHT / CRT_HEIGHT;
          float pixel_size = pixel_size_y < pixel_size_x ? pixel_size_y : pixel_size_x;
          DrawRectangle(x * pixel_size, y * pixel_size, pixel_size, pixel_size, color_converter(color));
        }
      }
      EndDrawing();
      // Inputs
      uint16_t key = (uint16_t)GetKeyPressed();
      if (key > 0) {
        printf("Key pressed: %d\n", key);
        *(uint16_t*)&memory[INPUT_KEY_ADDRESS] = key;
      }else if(IsKeyReleased(*(uint16_t*)&memory[INPUT_KEY_ADDRESS])){
        printf("Key Released: %d\n", *(uint16_t*)&memory[INPUT_KEY_ADDRESS]);
        *(uint16_t*)&memory[INPUT_KEY_ADDRESS] = 0;
      }
      last_draw = GetTime();
    }
  }

  // Clean up
  CloseWindow();
  hh_argparse_deinit(argparse);  
  return 0;
}
//-----------------------------------------------------------------------------
Color color_converter(uint8_t memcolor){
  switch (CRT_COLOR_MODE)
  {
  case COLOR_RGB332:
    return (Color){
      .r = (memcolor & 0xE0),
      .g = (memcolor & 0x1C) << 3,
      .b = (memcolor & 0x03) << 6,
      .a = 255
    };
  
  default:
    break;
  }
}
//-----------------------------------------------------------------------------
uint8_t read6502(uint16_t address) {
  return memory[address];
}
void write6502(uint16_t address, uint8_t value) {
  if(LOG_ON_WRITE_ADDRESS == address || address == 11 || address == 12){
    printf("PC: %04X\n", pc);
    printf("A: %02X, X: %02X, Y: %02X\n", a, x, y);
    printf("STATUS: %02X\n", status);
    printf("writing %02X to address: %04X\n", value, address);
    printf("-------------------\n");
  }
  memory[address] = value;
}
