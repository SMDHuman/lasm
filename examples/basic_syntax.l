// This example use instructions of 6502 cpu 
// use this example in lasm with as "lasm basic_syntax.l -cpu 6502"

// Zero page
zp {  		//if scope has a name at the start, it will persist and serves in that namespace
	<a 0>
	<b 2>
	<c 4>
}

<start[0x8000]>  // declare bracnh label and set assembler counter

<"hello_word.l"> // insert other file's content here

// Values
1234 // Decimal
0xff // Hexadecimal
0b11 // Binary

// Macro usage
inc_one zp.a
inc_one 2

// Curley brackets creates a scope limited for macros
// Outer macroes can be accessed inside but reverse can't happen
{  
	<hi 'hi'>
	<start 123>
	print hi, 2
	print_val start // prints value of 123
}
print_val start // prints value of 0x8000
// now 'hi' macro can't be accessable

jump start

<reset_vectors[0xfffa]> 
word_b; start; word_b

// Macroes
<byte_b; 0> // ";" is equevelent of new line
<word_b; byte_b; byte_b> // macros can be used in other macros

// Macro with argument
<inc_one <x>
	lda <x>
	adc #1
	sta <x>
>
<jump <there>
	lda #<there>[0] // project first byte of the value
	sta c
	lda# <there>[1] // '#' is a seperate token, the type of cpu may change the use of those characters  
	sta c+1         // Constant values can be processed in assembler with some math operations
	jmp c
>
