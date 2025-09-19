//-----------------------------------------------------------------------------
// This is a LASM file with subroutine insert macros. You need to put the 
// subroutine macros somewhere in memory.
// You can use MATH_IMPLEMENTATIONS to put your subroutines in a namepace like

// MATH_IMPLEMENTATIONS(MATH_ADDU16 MATH_MULTU8)

// To call subs after using MATH_IMPLEMENTATIONS
// ldx #number_a             // Load arguments
// ldy #number_b  
// stx math.add_u16.a        // Put arguments
// sty math.add_u16.b

// jsr math.add_u16          // Call function

// ldx math.add_u16.out      // 2 Byte out
// ldy math.add_u16.out + 1

// Last updated: 17/09/2025
//-----------------------------------------------------------------------------

< 1 MATH_ADDU16
  add_u16{out: a: 0.(2); b: 0.(2)}
  add_u16:{
    LDA add_u16.a      ; Load one operand into the accumulator.
    CLC                ; Clear the carry flag so it does not get added into the result
    ADC add_u16.b      ; Add the other operand
    STA add_u16.out    ; Store the operand back to x
    LDA add_u16.a+1    ; Now do the high byte
    ADC add_u16.b+1    ; Note we do not clear the carry this time.
    STA add_u16.out+1
    rts
  }
>  //MATH_ADDU16

< 1 MATH_MULTU8
  mult_u8{out: a: 0; b: 0}
  mult_u8:{
      LDA #0
      LDX  #0x8
      LSR  mult_u8.a
    loop:
      BCC  no_add
      CLC
      ADC  mult_u8.b
    no_add:
      ROR A
      ROR  mult_u8.a
      DEX
      BNE  loop
      STA  mult_u8.b
    rts
  }
>  //MATH_MULTU8

<MATH_IMPLEMENTATIONS(<subs>)
  math{
    <subs>
  }
> //MATH_IMPLEMENTATIONS