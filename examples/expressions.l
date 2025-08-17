
zp{
  a[0]: 
  b[2]:
  c[4]: 
  stack{
    start[256]:
    end[511]:
  }
}

macro_with_args 123+2, 412

{
start[16+zp.a]:
LABEL+65536
start - 0xa
LABEL:
2-(2+(1+(3+3)))*2+2
0b11.(10[3*(2-1)]) + 2
start
zp.c
"Hello"+3
zp.stack.end
LABEL 
macro
}

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
