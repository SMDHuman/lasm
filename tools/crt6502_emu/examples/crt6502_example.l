zp{
  pixel_address: 0.(2)
  x: 0
  y: 0
  color: 0xff 
}
<WIDTH 40>
<HEIGHT 30>
RESOLUTION[0x1000 - 4]: WIDTH; HEIGHT
KEY_INPUT[0x1000 - 2]: 0.(2)
DISPLAY_BUF[0x1000:: DISPLAY_BUF + (WIDTH * HEIGHT)]:

main[0x8000::0x9000]:
  {loop_y: 
    {loop_x:
      set_pixel_abs(zp.x, zp.y, zp.color)
      {
        lda zp.color
        clc; adc# 15
        sta zp.color
      }
      lda zp.x
      clc; adc# 1
      sta zp.x
      cmp# WIDTH
      bne loop_x
    }
    lda# 0
    sta zp.x
    lda zp.y
    clc; adc# 1
    sta zp.y
    cmp# HEIGHT
    bne loop_y
  }
  lda zp.color
  clc; adc# 1
  sta zp.color
  // Check if space key pressed
  lda KEY_INPUT
  cmp# 32
  beq exit_main
  jmp main
exit_main:brk


subs[0xA000::0xB000]{
  calculate_pixel_address{x:0;y:0}
  calculate_pixel_address:{
    lda calculate_pixel_address.y
    sta subs.multiply.a
    lda# WIDTH
    sta subs.multiply.b
    //...
    jsr subs.multiply
    //...
    lda subs.multiply.out
    adc calculate_pixel_address.x
    sta zp.pixel_address
    lda subs.multiply.out+1
    adc# 0
    sta zp.pixel_address+1
    // Add DISPLAY_BUF offset
    lda# DISPLAY_BUF & 0xff
    adc zp.pixel_address
    sta zp.pixel_address
    lda# (DISPLAY_BUF >> 8) & 0xff
    adc# 0
    adc zp.pixel_address+1
    sta zp.pixel_address+1
    rts
  }
  multiply{out: a: 0; b: 0}
  multiply:{
      LDA #0
      LDX  #0x8
      LSR  multiply.a
    loop:
      BCC  no_add
      CLC
      ADC  multiply.b
    no_add:
      ROR
      ROR  multiply.a
      DEX
      BNE  loop
      STA  multiply.b
    rts
  }
}

<set_pixel_imm (<x>, <y>, <color>)
  ldx# <x>
  ldy# <y>
  jsr subs.calculate_pixel_address
  lda# <color>
  ldx# 0
  sta (zp.pixel_address,x)
>
<set_pixel_abs (<x>, <y>, <color>)
  lda <x>
  sta subs.calculate_pixel_address.x
  lda <y>
  sta subs.calculate_pixel_address.y
  jsr subs.calculate_pixel_address
  lda <color>
  ldx# 0
  sta (zp.pixel_address,x)
>

reset_vectors[0xfffa]:
  0.(2);main;0xffff


