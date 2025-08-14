// fibonacci example for 6502 assembly
zp{
  a[0]:
  b[1]:
  c[2]:
}

start[256]:
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
  bcc loop
  clc; bcc HALT
}
0.[16]

HALT:
  clc; bcc HALT

reset_vectors[0x200]:
  0.[2];start;0.[2]
  