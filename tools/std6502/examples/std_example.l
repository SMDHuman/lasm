<"../std6502.l">

consts[0x4000]{
  hellow: "hello world\0"
}

start[0x8000]:
{
  emu_puts consts.hellow
  emu_puts_imm "Immidiate String\0"
}
emu_return 0

reset_vectors[0xfffa]:
  0.(2);start;0.(2)
