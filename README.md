## pip

<img width="1896" height="900" alt="image" src="https://github.com/user-attachments/assets/d80421aa-23eb-4544-a08c-551c50da4e39" />

a 16-bit virtual machine, built from scratch in C. it is has my own instruction set, my own assembler, and a live visualizer that shows every part of the machine as it runs.

### what is this?
pip is a tiny computer that doesn't physically exist. it's a program written in C that runs as a CPU. it has registers, memory, a stack, flags and a screen. it runs machine code in an instruction set i designed myself.  
programs for pip are written in pip's own assembly language turned into bytes by an assembler i wrote, and then run by the VM. the vm is compiled to WebAssembly so it runs in a browser, and the web page draws the whole machine live: what's in every register, which instruction is running, what the math unit is computing, where the program counter is in memory, which line of source code is executing, and which wires(not actual ones) the data is moving through.  
the biggest program running on it is flappy bird, written entirely in pip assembly.  
  
i built pip because i wanted to understand what actually happens inside a computer when code runs.  

### visualizer
when the page opens, pip loads the first program(which is pip.asm and it draws pip onto the screen) and auto runs it.  

#### controls
|control |what it does |
|--------|-------------|
|step |run exactly one instruction |
|run |run continously at speed |
|pause |stop running |
|reset |reload current program from the beginning |
|speed slider |from about 1 instruction per second up to 1.2 million per second. scale is exponential |

#### blocks
|block |what it shows |
|--------|-------------|
|rf(register file) |the 8 registers, R0 to R7 |
|ctrl(control) |the program counter. the three flags, and the next instuction |
|alu(math unit) |when the next instruction is arithmetic or logic, it shows the operation, the registers involved and the actual numbers. |
|stack |every return address currently on the stack |
|mem |a bar of all 64kb of memory split into code, data and stack with markers for the program counter(pc) and stack pointer(sp). under it, a zoomed strip of the program's own bytes where you can watch the pc move. then pip's 64x64 screen and a hex view of the bytes around the pc(the current instruction is in [])|
|i/o |output from PRINT, and hints for the current program |
|source |the assembly source of the running program with the line that's executing highlighted |

#### wires
the lines between the blocks are wires. when an instruction runs, the wires it uses light up and a pulse travels along them in the direction the data moves. at full speed you should see steady streams along whichever paths the program uses most.

#### custom mode
pick 'customs wowzers' and the source box turns into an editor. type pip assembly, press assemble and it's assembled right there in the browser(the assembler is compiled through WebAssembly too). then step through it or run it, with every block, wire and source line working exactly like the built-in programs. errors show up in the i/o box with a line number.

### programs
|program |what it does |
|--------|-------------|
|pip! |draws pip, the mascot, on the screen. traced from my own sketch. |
|flappy |flappy bird. you can play the game, the score freezes at 9. |
|fibonacci |prints the fibonacci sequence upto 20. |
|2+3 |adds 2 and 3. |
|langtons ant |an ant following two simple rules eventually builds a "highway". |
|call |shows CALL and RET. watch the stack block. |
|key |reacts to space/clicks |
|customs wowzers |write your own. |

### machine

#### registers and flags
- word size: 16 bits. every register holds a number from 0 to 65535.
- registers: 8 of them, R0 to R7. they're all general purpose, but R7 is the stack pointer. it starts at 0 and the stack grows downward into the stack region.
- flags: one byte, three flags.

|flag |bit |set when |
|-----|----|---------|
|overflow |1 |an arithmetic result didn't fit in 16 bits |
|zero |2 |an comparison found the two values equal |
|less |4 |an comparison found the first value smaller |
the conditional jumps read these flags.