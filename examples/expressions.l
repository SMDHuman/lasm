
zp{
  a[0]: 
  b[2]:
  c[4]: 
  stack{
    start[256]:
    end[511]:
  }
}
global:
things[123]:

{
start[17+zp.stack.end]:
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
macro_with_args 123+2, 412
}

global_out:

<macro
2*3.[5]
0x21 - 3 + 2
>

<macro_with_args <arg1> <arg2>
<arg1>
<arg2>
<arg1>
macro
>



// If uncommet this line, it will give an error
// becouse 'LABEL' initialized lower level of scope
// LABEL 
