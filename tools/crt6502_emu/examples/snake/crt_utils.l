<!WIDTH
    <WIDTH 40>
>
<!HEIGHT
    <HEIGHT 30>
>

<HALT_ADDRESS 0xffff>
<LOG_ON_WRITE_ADDRESS 0xFFBC>
RESOLUTION  [0x1000 - 4 :: 0x1000 - 2                    ]:
KEY_INPUT   [0x1000 - 2 :: 0x1000                        ]:
DISPLAY_BUF [0x1000     :: DISPLAY_BUF + (WIDTH * HEIGHT)]: 
    0.(WIDTH * HEIGHT)                                      // Just to see how much space buffer occupies after assembling

<UTIL_ZP
    pixel_address: 0.(2)
    r_seed: 0
>
<UTIL_SETUP
{
    SET_MEM_IMM zp.r_seed, 1
    SET_MEM_IMM RESOLUTION, WIDTH
    SET_MEM_IMM RESOLUTION+1, HEIGHT
}
>
<UTIL_IMPLEMENTATION
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
      ROR a
      ROR  multiply.a
      DEX
      BNE  loop
      STA  multiply.b
    rts
  }
>
<set_pixel_imm <x> <y> <color>
  lda# <x>
  sta subs.calculate_pixel_address.x
  lda# <y>
  sta subs.calculate_pixel_address.y
  jsr subs.calculate_pixel_address
  lda# <color>
  ldx# 0
  sta (zp.pixel_address,x)
>
<set_pixel_abs <x> <y> <color>
  lda <x>
  sta subs.calculate_pixel_address.x
  lda <y>
  sta subs.calculate_pixel_address.y
  jsr subs.calculate_pixel_address
  lda <color>
  ldx# 0
  sta (zp.pixel_address,x)
>
<SET_MEM_IMM <addrs> <val>
  lda#<val>;sta <addrs>
>
<RGB332 <r> <g> <b>
  (<r> & 0x7) << 5 | (<g> & 0x7) << 2 | (<b> & 0x3)
>