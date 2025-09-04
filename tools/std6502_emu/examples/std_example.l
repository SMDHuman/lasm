<"../std6502.l">

zp[0::256]{
  return_buf: 0.(16)
}

consts[0x4000]{
  hellow: "hello world\0"
}

start[0x8000::start + 160]:
{
  emu_puts (consts.hellow)
  emu_putchar ("a")
  emu_putchar ("b")
  emu_putchar ("\n")
  emu_puts_imm ("Enter something: \0")
  {
    loop:
    emu_getchar
    lda emu_return_address
    cmp #"\n"
    beq exit
    emu_putchar_a
    emu_putchar ("\n")
    jmp loop
    exit:
  }
}
emu_return (0)

sub[0xA000 :: 0xB000]{
  copy_return_buffer:{
    ldx# 16
      loop:
      lda emu_return_address, x
      sta zp.return_buf, x
      dex
      bne loop
    rts
  }
}

reset_vectors[0xfffa]:
  0.(2);start;0.(2)
