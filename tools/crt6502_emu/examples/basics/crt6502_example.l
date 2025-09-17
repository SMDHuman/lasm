zp{
  UTIL_ZP
  x: 0
  y: 0
  color: 0xff 
}
<WIDTH 160>
<HEIGHT 120>
<"crt_utils.l">

main[0x8000::0x9000]:
  UTIL_SETUP
main_loop:
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
  lda# 0
  sta zp.y
  lda zp.color
  clc; adc# 1
  sta zp.color
  // Check if space key pressed
  lda KEY_INPUT
  cmp# 32
  beq exit_main
  jmp main_loop
exit_main:brk


subs[0xA000::0xB000]{
  UTIL_IMPLEMENTATION
}

reset_vectors[0xfffa]:
  0.(2);main;0xffff


