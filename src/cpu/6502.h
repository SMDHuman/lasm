
#ifndef cpu6502_H
#define cpu6502_H

#include <stdint.h>
#include "../lasm_machine.h"

lasm_mcode_recipe_t lasm_6502_recipes[] = {
  // Instructions recipe,     opcode, bytes
  {"ADC _#_ _BYTE_ _;_", 0x69, 2},
  {"ADC _BYTE_ _;_", 0x65, 2},
  {"ADC _BYTE_ _,_ X _;_", 0x75, 2},
  {"ADC _WORD_ _;_", 0x6D, 3},
  {"ADC _WORD_ _,_ X _;_", 0x7D, 3},
  {"ADC _WORD_ _,_ Y _;_", 0x79, 3},
  {"ADC _(_ _BYTE_ _,_ X _)_ _;_", 0x61, 2},
  {"ADC _(_ _BYTE_ _)_ _,_ Y _;_", 0x71, 2},

  {"AND _#_ _BYTE_ _;_", 0x29, 2},
  {"AND _BYTE_ _;_", 0x25, 2},
  {"AND _BYTE_ _,_ X _;_", 0x35, 2},
  {"AND _WORD_ _;_", 0x2D, 3},
  {"AND _WORD_ _,_ X _;_", 0x3D, 3},
  {"AND _WORD_ _,_ Y _;_", 0x39, 3},
  {"AND _(_ _BYTE_ _,_ X _)_ _;_", 0x21, 2},
  {"AND _(_ _BYTE_ _)_ _,_ Y _;_", 0x31, 2},

  {"ASL A _;_", 0x0A, 1},
  {"ASL _BYTE_ _;_", 0x06, 2},
  {"ASL _BYTE_ _,_ X _;_", 0x16, 2},
  {"ASL _WORD_ _;_", 0x0E, 3},
  {"ASL _WORD_ _,_ X _;_", 0x1E, 3},

  {"BCC _RELBYTE_ _;_", 0x90, 2},

  {"BCS _RELBYTE_ _;_", 0xB0, 2},

  {"BEQ _RELBYTE_ _;_", 0xF0, 2},

  {"BIT _BYTE_ _;_", 0x24, 2},
  {"BIT _WORD_ _;_", 0x2C, 3},

  {"BMI _RELBYTE_ _;_", 0x30, 2},

  {"BNE _RELBYTE_ _;_", 0xD0, 2},

  {"BPL _RELBYTE_ _;_", 0x10, 2},

  {"BRK _;_", 0x00, 1},

  {"BVC _RELBYTE_ _;_", 0x50, 2},

  {"BVS _RELBYTE_ _;_", 0x70, 2},

  {"CLC _;_", 0x18, 1},

  {"CLD _;_", 0xD8, 1},

  {"CLI _;_", 0x58, 1},

  {"CLV _;_", 0xB8, 1},

  {"CMP _#_ _BYTE_ _;_", 0xC9, 2},
  {"CMP _BYTE_ _;_", 0xC5, 2},
  {"CMP _BYTE_ _,_ X _;_", 0xD5, 2},
  {"CMP _WORD_ _;_", 0xCD, 3},
  {"CMP _WORD_ _,_ X _;_", 0xDD, 3},
  {"CMP _WORD_ _,_ Y _;_", 0xD9, 3},
  {"CMP _(_ _BYTE_ _,_ X _)_ _;_", 0xC1, 2},
  {"CMP _(_ _BYTE_ _)_ _,_ Y _;_", 0xD1, 2},

  {"CPX _#_ _BYTE_ _;_", 0xE0, 2},
  {"CPX _BYTE_ _;_", 0xE4, 2},
  {"CPX _WORD_ _;_", 0xEC, 3},

  {"CPY _#_ _BYTE_ _;_", 0xC0, 2},
  {"CPY _BYTE_ _;_", 0xC4, 2},
  {"CPY _WORD_ _;_", 0xCC, 3},

  {"DEC _BYTE_ _;_", 0xC6, 2},
  {"DEC _BYTE_ _,_ X _;_", 0xD6, 2},
  {"DEC _WORD_ _;_", 0xCE, 3},
  {"DEC _WORD_ _,_ X _;_", 0xDE, 3},

  {"DEX _;_", 0xCA, 1},

  {"DEY _;_", 0x88, 1},

  {"EOR _#_ _BYTE_ _;_", 0x49, 2},
  {"EOR _BYTE_ _;_", 0x45, 2},
  {"EOR _BYTE_ _,_ X _;_", 0x55, 2},
  {"EOR _WORD_ _;_", 0x4D, 3},
  {"EOR _WORD_ _,_ X _;_", 0x5D, 3},
  {"EOR _WORD_ _,_ Y _;_", 0x59, 3},
  {"EOR _(_ _BYTE_ _,_ X _)_ _;_", 0x41, 2},
  {"EOR _(_ _BYTE_ _)_ _,_ Y _;_", 0x51, 2},

  {"INC _BYTE_ _;_", 0xE6, 2},
  {"INC _BYTE_ _,_ X _;_", 0xF6, 2},
  {"INC _WORD_ _;_", 0xEE, 3},
  {"INC _WORD_ _,_ X _;_", 0xFE, 3},

  {"INX _;_", 0xE8, 1},

  {"INY _;_", 0xC8, 1},

  {"JMP _WORD_ _;_", 0x4C, 3},
  {"JMP _(_ _WORD_ _)__;_", 0x6C, 3},

  {"JSR _WORD_ _;_", 0x20, 3},

  {"LDA _#_ _BYTE_ _;_", 0xA9, 2},
  {"LDA _BYTE_ _;_", 0xA5, 2},
  {"LDA _BYTE_ _,_ X _;_", 0xB5, 2},
  {"LDA _WORD_ _;_", 0xAD, 3},
  {"LDA _WORD_ _,_ X _;_", 0xBD, 3},
  {"LDA _WORD_ _,_ Y _;_", 0xB9, 3},
  {"LDA _(_ _BYTE_ _,_ X _)_ _;_", 0xA1, 2},
  {"LDA _(_ _BYTE_ _)_ _,_ Y _;_", 0xB1, 2},

  {"LDX _#_ _BYTE_ _;_", 0xA2, 2},
  {"LDX _BYTE_ _,_ Y _;_", 0xB6, 2},
  {"LDX _WORD_ _,_ Y _;_", 0xBE, 3},

  {"LDY _#_ _BYTE_ _;_", 0xA0, 2},
  {"LDY _BYTE_ _,_ X _;_", 0xB4, 2},
  {"LDY _WORD_ _,_ X _;_", 0xBC, 3},

  {"LSR A _;_", 0x4A, 1},
  {"LSR _BYTE_ _;_", 0x46, 2},
  {"LSR _BYTE_ _,_ X _;_", 0x56, 2},
  {"LSR _WORD_ _;_", 0x4E, 3},
  {"LSR _WORD_ _,_ X _;_", 0x5E, 3},

  {"NOP _;_", 0xEA, 1},

  {"ORA _#_ _BYTE_ _;_", 0x09, 2},
  {"ORA _BYTE_ _;_", 0x05, 2},
  {"ORA _BYTE_ _,_ X _;_", 0x15, 2},
  {"ORA _WORD_ _;_", 0x0D, 3},
  {"ORA _WORD_ _,_ X _;_", 0x1D, 3},
  {"ORA _WORD_ _,_ Y _;_", 0x19, 3},
  {"ORA _(_ _BYTE_ _,_ X _)_ _;_", 0x01, 2},
  {"ORA _(_ _BYTE_ _)_ _,_ Y _;_", 0x11, 2},

  {"PHA _;_", 0x48, 1},

  {"PHP _;_", 0x08, 1},

  {"PLA _;_", 0x68, 1},

  {"PLP _;_", 0x28, 1},

  {"ROL A _;_", 0x2A, 1},
  {"ROL _BYTE_ _;_", 0x26, 2},
  {"ROL _BYTE_ _,_ X _;_", 0x36, 2},
  {"ROL _WORD_ _;_", 0x2E, 3},
  {"ROL _WORD_ _,_ X _;_", 0x3E, 3},

  {"ROR A _;_", 0x6A, 1},
  {"ROR _BYTE_ _;_", 0x66, 2},
  {"ROR _BYTE_ _,_ X _;_", 0x76, 2},
  {"ROR _WORD_ _;_", 0x6E, 3},
  {"ROR _WORD_ _,_ X _;_", 0x7E, 3},

  {"RTI _;_", 0x40, 1},

  {"RTS _;_", 0x60, 1},

  {"SBC _#_ _BYTE_ _;_", 0xE9, 2},
  {"SBC _BYTE_ _;_", 0xE5, 2},
  {"SBC _BYTE_ _,_ X _;_", 0xF5, 2},
  {"SBC _WORD_ _;_", 0xED, 3},
  {"SBC _WORD_ _,_ X _;_", 0xFD, 3},
  {"SBC _WORD_ _,_ Y _;_", 0xF9, 3},
  {"SBC _(_ _BYTE_ _,_ X _)_ _;_", 0xE1, 2},
  {"SBC _(_ _BYTE_ _)_ _,_ Y _;_", 0xF1, 2},

  {"SEC _;_", 0x38, 1},

  {"SED _;_", 0xF8, 1},

  {"SEI _;_", 0x78, 1},

  {"STA _BYTE_ _;_", 0x85, 2},
  {"STA _BYTE_ _,_ X _;_", 0x95, 2},
  {"STA _WORD_ _;_", 0x8D, 3},
  {"STA _WORD_ _,_ X _;_", 0x9D, 3},
  {"STA _WORD_ _,_ Y _;_", 0x99, 3},
  {"STA _(_ _BYTE_ _,_ X _)_ _;_", 0x81, 2},
  {"STA _(_ _BYTE_ _)_ _,_ Y _;_", 0x91, 2},

  {"STX _BYTE_ _;_", 0x86, 2},
  {"STX _BYTE_ _,_ Y _;_", 0x96, 2},
  {"STX _WORD_ _;_", 0x8E, 3},

  {"STY _BYTE_ _;_", 0x84, 2},
  {"STY _BYTE_ _,_ X _;_", 0x94, 2},
  {"STY _WORD_ _;_", 0x8C, 3},

  {"TAX _;_", 0xAA, 1},

  {"TAY _;_", 0xA8, 1},

  {"TSX _;_", 0xBA, 1},

  {"TXA _;_", 0x8A, 1},

  {"TXS _;_", 0x9A, 1},

  {"TYA _;_", 0x98, 1}
};







#endif /* cpu6502_H */