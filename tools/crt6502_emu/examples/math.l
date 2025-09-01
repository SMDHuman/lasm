
// Place this where you want to place subroutines of this module
<subroutine_implementations
  math{
    add_uint16{out: a: 0.(2); b: 0.(2)}
    add_uint16:{
      rts
    }
    multiply_uint8{out: a: 0; b: 0}
    multiply_uint8:{
        LDA #0
        LDX  #0x8
        LSR  multiply.a
      loop:
        BCC  no_add
        CLC
        ADC  multiply.b
      no_add:
        ROR a
        ROR  multiply.a
        DEX
        BNE  loop
        STA  multiply.b
      rts
    }
  }

>//subroutine_implementations