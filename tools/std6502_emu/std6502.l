<emu_call_func 0x1100>
<emu_return_address 0x1101>

<emu_return (<exit_code>)
  pha
  {
    lda# 6
    sta emu_call_func
    jmp next
    0x00.2; (<exit_code>).(4)
    next:
  }
  pla
>
<emu_puts (<str_pointer>)
  pha
  {
    lda# 4
    sta emu_call_func
    jmp next
    0x0001.(2); (<str_pointer>).(2); 
    next:
  }
  pla
>
<emu_puts_imm (<str>)
  {
    emu_puts (imm_str)
    jmp next
    imm_str: <str>;
    next:
  }
>
<emu_getchar
  pha
  {
    lda# 2
    sta emu_call_func
    jmp next
    0x0002.(2) 
    next:
  }
  pla
>
<emu_putchar (<char>)
  pha
  {
    lda# 3
    sta emu_call_func
    jmp next
    0x0003.(2); (<char>).(1); 
    next:
  }
  pla
>
<emu_putchar_a
  pha
  {
    sta next - 1
    lda# 3
    sta emu_call_func
    jmp next
    0x0003.(2); "n".(1); 
    next:
  }
  pla
>
