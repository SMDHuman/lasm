
zp{
  a[0]: 
  b[2]:
  c[4]: 
  stack[0x100]{
    start:
    end[start+255]:
  }
}
global:
things[123]:

{
start[17+zp.stack.end]:
(0b11010011 & 0xf0) >> 4 | 0x10
start + 1
global_out
LABEL -1
LABEL:
2-(2+(1+(3+3)))*2+2
0b11.(0x200[1*(3-2)]) + 2
start
zp.c
"Hello"+3
zp.stack.end
LABEL 
macro_with_args (123+2, 412)
}

global_out:

<macro
2*3.[5]
0x21 - 3 + 2
>

<macro_with_args (<arg1>, <arg2>)
<arg1>
<arg2>
<arg1>
macro
>

<abc 0>
<efg abc>

efg

// If uncommet this line, it will give an error
// becouse 'LABEL' initialized lower level of scope
// LABEL 
