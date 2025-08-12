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
0b11
start
zp.c
zp.stack.end
LABEL
}

lda x

// If uncommet this line, it will give an error
// becouse 'LABEL' initialized lower level of scope
// LABEL 
