data[0x4000]:
consts{
  hellow: "hello word";
}


start[0x8000]:
{
  emu_puts consts.hellow
}
emu_return 0

<call_func 0x1100>

<emu_return <exit_code>
  lda# 6
  {
    sta call_func
    jmp next
    0x00.2; (<exit_code>).(4)
    next:
  }
>

<emu_puts <str_pointer>
  lda# 4
  {
    sta call_func
    jmp next
    0x0001.(2); (<str_pointer>).(2); 
    next:
  }
>


reset_vectors[0xfffa]:
  0.(2);start;0.(2)
