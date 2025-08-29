data[0x4000]:
consts{
  hellow: "hello word";
}



start[0x8000]:
{
  emu_puts consts.hellow
  { 
    bvc str_end
    str: "another string" ;0; str_end: 
    emu_puts str
  }
}
emu_return

reset_vectors[0xfffa]:
  0.(2);start;0.(2)

<emu_puts <str_pointer>
  lda# <str_pointer>[0]
  sta 0x1120
  lda# <str_pointer>[1]
  sta 0x1121
  lda# 01
  sta 0x1100
  lda# 00
  sta 0x1101
>  

<emu_return
  lda# 00
  sta 0x1100
  lda# 00
  sta 0x1101
>