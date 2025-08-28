// 6502 Assembly - Addressing Modes Example

// Implied
CLC             // Clear Carry flag
SEC             // Set Carry flag

// Accumulator
ASL A           // Shift Left Accumulator
ROL A           // Rotate Left Accumulator

// Immediate
LDA #0x10        // Load Accumulator with immediate value 0x10
ADC #0x05        // Add with Carry immediate value 0x05

// Zero Page
LDA 0x80         // Load Accumulator from zero page address 0x80
STA 0x81         // Store Accumulator to zero page address 0x81

// Zero Page,X
LDA 0x90,X       // Load Accumulator from zero page address 0x90+X
STA 0x91,X       // Store Accumulator to zero page address 0x91+X

// Zero Page,Y (limited instructions support this)
LDX 0xA0,Y       // Load X from zero page address 0xA0+Y
STX 0xA1,Y       // Store X to zero page address 0xA1+Y

// Absolute
JMP 0x1000       // Jump to address 0x1000
JSR 0x2000       // Jump to Subroutine at address 0x2000

// Absolute,X
LDA 0x3000,X     // Load Accumulator from address 0x3000+X
STA 0x3100,X     // Store Accumulator to address 0x3100+X

// Absolute,Y
LDA 0x4000,Y     // Load Accumulator from address 0x4000+Y
STA 0x4100,Y     // Store Accumulator to address 0x4100+Y

// Indirect
JMP (0x5000)     // Jump to address stored at 0x5000 and 0x5001

// Indirect,X (Pre-indexed)
LDA (0x60,X)     // Load Accumulator from address stored at (0x60+X)
STA (0x61,X)     // Store Accumulator to address stored at (0x61+X)

// Indirect,Y (Post-indexed)
LDA (0x70),Y     // Load Accumulator from address stored at 0x70 plus Y
STA (0x71),Y     // Store Accumulator to address stored at 0x71 plus Y

// Relative (for branch instructions)
loop:
  BEQ end    // Branch to loop if equal (Z=1)
  BNE loop    // Branch to loop if not equal (Z=0)

end:

// Output of this file should be 
//18     // CLC
//38     // SEC

//0A     // ASL A
//2A     // ROL A

//A9 10  // LDA #$10
//69 05  // ADC #$05

//A5 80  // LDA $80
//85 81  // STA $81

//B5 90  // LDA $90,X
//95 91  // STA $91,X

//B6 A0  // LDX $A0,Y
//96 A1  // STX $A1,Y

//4C 00 10  // JMP $1000
//20 00 20  // JSR $2000

//BD 00 30  // LDA $3000,X
//9D 00 31  // STA $3100,X

//B9 00 40  // LDA $4000,Y
//99 00 41  // STA $4100,Y

//6C 00 50  // JMP ($5000)

//A1 60  // LDA ($60,X)
//81 61  // STA ($61,X)

//B1 70  // LDA ($70),Y
//91 71  // STA ($71),Y

//F0 FE  // BEQ loop (-2 bytes relative branch)
//D0 FC  // BNE loop (-4 bytes relative branch)
