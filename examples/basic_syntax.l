// This example use instructions of 6502 cpu 
// use this example in lasm with as "lasm basic_syntax.l -cpu 6502"

// Zero page
//if scope has a name at the start, it will persist and serves in that namespace
zp {  	
	a[0]:
	b[2]:
	c[4]:
}

asd

<asd dsa>
<dsa 123>


start[0x8000]:  // declare bracnh label and set assembler counter

<"hello_world.l"> // insert other file's content here

// Values
1234 // Decimal number size of 2 byte
0xff // Hexadecimal number size of 1 bytes
0b11 .4 // Binary number size of 4 bytes

// Macro usage
{
again:
inc_one zp.a
inc_one 2     // zp.b
beq again     // Relative jump
}
// Curley brackets creates a scope limited for macros
// Outer macroes can be accessed inside but reverse can't happen
{  
	print_str hi, 2
	print_val start // prints value of 123
	jump out
	hi: 'hi'
	start: 123
	out:
}
print_val start // prints value of 0x8000
// now 'hi' macro can't be accessable

jump start

reset_vectors[0xfffa]:
0.2; start; 0.2

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
<print_str <ptr> <size>
// things
>

<print_val <num>
// things
>

// 'a' used as a macro label to replace every 'a' with '32'
<ax 32>

// If 'a' is not used as a macro label, insert the code to current position
<!ax 
 	<ax 23>
 >

// If 'a' is used as a macro label, insert the code to current position
 <?ax
	lda #ax
 >
