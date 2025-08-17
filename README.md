# lotp-asm
LOTP assembler is a high-level assembler for my custom CPUs and the 6502 CPU. The purpose of this project is for me to learn and add some features of my own.

### What has special than other assemblers?
It has all the necessary macros that we love from the C language, namespaces for branch labels, and support for numbers of arbitrary (effectively unlimited) size. All of these are to make this language fun to use and to try new methods of writing assembly.

### What "LOTP" means?
LOTP is an acronym that I use to give my project's name as a signature of mine. It means "Line On The Paper". From the start of my engineering hobby, I usually start a project on a paper with some sketches and ideas. It refers to the root branch of the start of the project.

___
## Features 
### Comments and New Lines
  You can write comments with the good old ```//``` double slash. Usually, you do not need to use ```;``` at the end of your lines. New lines are sufficient, but if you want to write multiple statements on one line, you can separate them with semicolons.
  ```
  // My comment on this code is NULL
  lda# 3; sta 0
  adc 1; sta 1 //Things happen
  //adc 1; sta 3 Things will not happen
  // End of Code  
  ```
### Macros
Macros are defined always inside ```< this >``` structure. There are 4 main types of macros. Include, Define, Define With Arguments and If Define, as we know them from C89.

* This will `INCLUDE` the entire code from that file to its current position
  ```
  <"path/to/filename.l">
  ```
* When you `DEFINE` a macro like this, whenever assembler sees 'recall_name', it will replace it with what comes after the macro's definition.
  ```
  <recall_name and other things>
  ```
  
* `DEFINE WITH ARGS` macro is identical to normal define macro, but there is some positional arguments that you can call like ```recall_name salam, sausage```
  ```
  <recall_name <arg_name> <another_arg>
    and other things <arg_name> with <another_arg>
  >
  ```

* `IF DEFINE` is a controller for macro parser to enable certain portion of code or block it
  ```
  <?this
    can do this   // if 'this' is defined, it will do 'can do'
  >
  <!that
    <that define> // if 'that' not defined, define with 'define'
  >
  ```
### Everything is data
  Assemblers almost only purpose to convert keywords and number representations into list of bytes to upload to a memory for execution. Everything that doesn't belong to instruction words will be parsed as a sort of expression.
  ```
  123; 0xfe; 0b01011
  3 + (2 - 3) * (0x12 << 2) / 3
  "Hello Word Again"
  ```
  You can also manipulate the values with some special operations
  ```
  "text of short"[2] // Isolates character 'x' by the index 
  0xfff .[2] // this will ensure the value takes specifically 2 bytes of space
  ```
  LASM will allocate enough memory for each expression to fit its largest possible value
  ```
  3 + 2.[32] - 1 // This is equals to 4 but occupies 32 bytes of space
  ```
### Branch Labels
  Branch labels have two properties that you need to consider while using: has it a determined value? and is it evaluated now? Branches can be used anywhere in the code. You can call future branches from previous lines. But some instructions or features want to know all values specifically. And undetermined labels are evaluated when the assembler parses them.
  ```
  x[0]: // Those are determined labels
  y[x+1]: // So you can use them anywhere as constant values
  z[y+2]:


  // somewhere[loop - 32]: LASM will report an error for this
  // Cause we don't know where the 'loop' is right now

  stack[start - 32]: // But this is ok cause start is determined

  start[0x8000]:
    lda # 42
    loop: // you can use this branch values most of the time
      sta x
      adc 1
      bcc loop
    jmp end

  end[0xff00 + loop]: // This is also ok cause at this point we encounter with 'loop'
    hlt
  ```
> **_NOTE:_** Determined labels can be any size of byte but undetermined labels fixed to machine specific byte size.