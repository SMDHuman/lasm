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
<emu_puts_imm <str>
  {
    emu_puts imm_str
    jmp next
    imm_str: <str>;
    next:
  }
>
