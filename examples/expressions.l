
zp{
  a[0]: 
  b[2]:
  c[4]: 
  stack{
    start[256]:
    end[511]:
  }
}

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
}


// If uncommet this line, it will give an error
// becouse 'LABEL' initialized lower level of scope
// LABEL 
