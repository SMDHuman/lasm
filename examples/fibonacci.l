// fibonacci example for 6502 assembly
zp{
  a[0]:
  b[1]:
  c[2]:
}

start[0x8000]:
// initialize 
lda #1; sta zp.a

//start loop
{
  loop:
  // c = a + b
  lda zp.a
  adc zp.b
  sta zp.c
  // a = b
  lda zp.b
  sta zp.a
  // b = c
  lda zp.c
  sta zp.b
  // wait input
  lda #1
  sta 0x1100
  // loop back or exit on carry set
  bcc loop
  clc; bcc HALT
}
0.[16]

HALT:
  lda #0
  sta 0x1100


reset_vectors[0xfffa]:
  0.(2);start;HALT
  