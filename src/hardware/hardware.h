//-----------------------------------------------------------------------------
// cpu/hardware.h
// Header for CPU hardware definitions and functions
//-----------------------------------------------------------------------------

#ifndef CPU_HARDWARE_H
#define CPU_HARDWARE_H

#include <stdint.h>
#include "lasm_assembler.h"

typedef struct{
    const char* name;
    void (*initializer)(assembler_t *assembler);
    uint8_t (*assembler)(assembler_t *assembler);
} hardware_t;


//-----------------------------------------------------------------------------
// HARDWARE DEFINITIONS
//-----------------------------------------------------------------------------

// INCLUDE CPU IMPLEMENTATIONS HERE 
#include "6502.c"

// DEFINE HARDWARES HERE
hardware_t hardwares[] = {
    {"6502", lasm_6502_init, (uint8_t (*)(assembler_t *))lasm_6502_assemble}
};

//-----------------------------------------------------------------------------
#endif // CPU_HARDWARE_H