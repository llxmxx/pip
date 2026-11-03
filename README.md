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

#### instruction format
every instruction is one 16-bit word. LOADI takes a second word holding a constant.
- opcode(5 bits): which instruction. 5bits allows 32; pip uses 28/
- dest, src1, src2(3 bits each): register numbers 0 to 7.
- last 2 bits are spare
- words are stored big-endian: high byte first

#### instruction set
|opcode |instruction |form |meaning |
|-------|------------|-----|--------|
|0 |HALT |HALT |stop machine |
|1 |ADD |ADD Rd, Ra, Rb |Rd = Ra + Rb, sets overflow if it doesn't fit |
|2 |SUB |SUB Rd, Ra, Rb |Rd = Ra - Rb |
|3 |MUL |MUL Rd, Ra, Rb |Rd = Ra * Rb, sets overflow if it doesn't fit |
|4 |DIV |DIV Rd, Ra, Rb |Rd = Ra / Rb, error if Rb is 0 |
|5 |LOADI |LOADI Rd, value |put a constant or label address into Rd |
|6 |CMP |CMP Ra, Rb |compare, set zero/less flags |
|7 |JMP |JMP Rt |jump to address in Rt |
|8 |JZ |JZ Rt, Rv |jump to Rt if Rv is 0 |
|9 |JNZ |JNZ Rt, Rv |jump to Rt if Rv is not 0 |
|10 |JEQ |JEQ Rt |jump if equal |
|11 |JNE |JNE Rt |jump if not equal |
|12 |JLT |JLT Rt |jump if less |
|13 |JGT |JGT Rt |jump if greater |
|14 |JLE |JLE Rt |jump if less or equal |
|15 |JGE |JGE Rt |jump if greater or equal |
|16 |LOAD |LOAD Rd, [Ra] |read word at address in Ra into Rd |
|17 |STORE |STORE [Ra], Rv |write Rv to address in Ra |
|18 |MOV |MOV Rd, Ra |Rd = Ra |
|19 |CALL |CALL Rt |push return address, jump to Rt |
|20 |RET |RET |pop return address, jump back |
|21 |AND |AND Rd, Ra, Rb |Rd = Ra & Rb |
|22 |OR |OR Rd, Ra, Rb |Rd = Ra \| Rb |
|23 |XOR |XOR Rd, Ra, Rb |Rd = Ra ^ Rb |
|24 |NOT |NOT Rd, Ra |Rd = ~Ra |
|25 |SHL |SHL Rd, Ra, Rb |Rd = Ra << Rb |
|26 |SHR |SHR Rd, Ra, Rb |Rd = Ra >> Rb |
|27 |PRINT |PRINT Ra |print value of Ra |

#### memory map
64kb in total, split into three regions:
- code(24kb), program is loaded here from address 0
- data(24kb), variables plus the screen and i/o words
- stack(16kb), return addresses; R7 grows fown from the top

inside the data region:
|address |what it is |
|--------|-----------|
|0x6000 to 0x7FFF |the screen(64x64 pixels, one 16-bit word each) |
|0xBFFC |frame counter, written by the page every tick |
|0xBFFE |keyboard word, set to 1 when space or a click happens |

#### memory-mapped i/o
pip has no graphics or input instructions. everything goes through memory, the way real hardware often does.
- screen: 64x64 pixels. pixel (x, y) lives at 0x6000 + (y * 64 + x) * 2 and holds a color in RGB565 (5 bits red, 6 green, 5 blue). STORE a color there and the pixel changes. every frame, the page reads the whole region and paints it.
- clock: about 60 times a second the page writes a frame counter into 0xBFFC. a program that wants a steady speed (like a game) waits for that value to change before drawing the next frame.
- keyboard: when you press space or click the screen, the page writes 1 into 0xBFFE. the program reads it, reacts, and writes 0 back.

#### errors
the vm stops with an error instead of doing something weird:
- dividing by zero
- writing into the code region (programs can't overwrite themselves)
- the program counter leaving the code region
- stack overflow on CALL, or RET with nothing on the stack

error messages report the address of the instruction that failed.

### assembler
the assembler turns readable assembly into pip's bytes.
```
; counts 0 to 5 and prints each number
LOADI R1, 0
LOADI R2, 1
LOADI R3, 6
LOADI R6, loop
loop:
PRINT R1
ADD R1, R1, R2
CMP R1, R3
JLT R6
HALT
```
it works in two passes:
1. pass 1 walks the file and records the address of every label, counting 2 bytes per instruction and 4 for LOADI.
2. pass 2 walks it again and encodes each instruction, now able to swap label names for their addresses, even labels defined later in the file.

syntax:
- one instruction per line; ; starts a comment.
- labels end with : and sit on their own line.
- registers are R0 to R7. commas and [] are just for readability.
- numbers can be decimal or hex (0xA004). only LOADI takes a number or label.

errors come with line numbers, like "line 4: ADD needs 3 operands" or "unknown instruction FOO on line 2".

outputs:
- program.bin: the bytes the vm runs.
- program.bin.map: one line per instruction, "address line". the visualizer uses this to highlight the source line that's running.

there are two builds of the same assembler: asm.c (command line, reads and writes files) and asm_web.c (reads text and writes into memory, compiled into the page for custom mode).

### writing your own program
- constants and addresses always go through LOADI. to add 1: LOADI R5, 1 then ADD R3, R3, R5.
- jumps need their target in a register first: LOADI R6, done then JEQ R6.
- to use a variable, pick an address in the data region (like 0xA000), LOADI it into a register, then LOAD/STORE through it.
- functions are just labels you CALL that end with RET. there's no "save registers" instruction, so decide which registers a function is allowed to wreck and reload them after the call. (in flappy, the drawing helpers wreck R5 and R6, so every caller reloads those after.)
- to draw a pixel, compute 0x6000 + (y * 64 + x) * 2 and STORE a color there. shifting left by 6 multiplies by 64.

the fastest way to try things is custom on the live page.

### how flappy bird works
flappy.asm is the biggest program on pip, and everything in it is built from the instructions above.

|where |what |
|------|-----|
|R0 |the bird's height in sixteenths of a pixel (draw row = R0 >> 4). this fixed-point trick gives smooth motion with integers only |
|R1 |the last frame number seen |
|0xA002 |vertical velocity. gravity adds 1 every frame, capped at 24; a flap sets it to 0xFFEE (-18) |
|0xA004 |the pipe's x position, counting down from 60 to 0, then wrapping |
|0xA006 |top of the gap. the gap is 18 rows tall |
|0xA00C |random seed |
|0xA00E |score |
|0xA020+ |digit shapes for the score |

the game loop, once per frame:
1. wait for the frame counter at 0xBFFC to change
2. erase the bird (redraw it in sky color)
3. apply gravity to the velocity, and the velocity to the height
4. check for death: hitting the ground, or touching the pipe outside its gap
5. check the keyboard word; if pressed, flap and clear it
6. draw the bird at its new height
7. every other frame, erase the pipe, move it one column left, and redraw it
8. draw the score

random gaps: each new pipe gets a random gap height from a linear congruential generator:
```
seed = seed * 25173 + 13849      (16-bit, wraps on its own)
gap  = (seed >> 11) + 5          (top 5 bits -> 0..31, shifted into a safe range)
```
the top bits are used because the low bits of this kind of generator are weak. the seed is mixed with the frame counter at the moment of the first flap, and since that depends on a human, every game is different.

scoring: a point is added when the pipe's x reaches 12. that's when its right edge (x + 3) has fully passed the bird (column 16). the pipe moves one column at a time, so this happens exactly once per pipe.

the score digits: each digit is a 3x5 pixel shape. that's 15 pixels, so it fits in one 16-bit word, one bit per pixel. to draw a digit, the program looks up its shape, then for each pixel shifts the word right and ANDs with 1 to see if it's on.

death: the bird turns red where it hit, the score stays, and the next flap restarts.

### how the visualizer works
- the vm is the same C code as the terminal version, compiled to WebAssembly with emscripten. web.c exposes small functions the page can call: reset, load a byte, step, run n instructions, read a register/the pc/the flags/a memory byte, poke a memory byte.
- the run loop (app.js) ticks about 60 times a second. each tick it writes the frame counter, works out how many instructions to run from the speed slider (keeping fractional leftovers, so 1 instruction per second really means 1 per second), runs them, and redraws everything.
- the screen is a canvas. each tick, the page reads the 4096 pixel words and paints them.
- the alu box decodes the next instruction straight from memory and, if it's arithmetic, computes a preview of the result.
- the source panel looks the pc up in the .bin.map file to find which line to highlight.
- the wires are svg lines between the centers of neighbouring blocks, drawn behind the blocks so only the gaps show. each opcode has a list of wires it uses and which way the data flows; a pulse is one short glowing dash animated along the line.
- custom mode calls the in-browser assembler, loads the bytes it returns into the vm, and builds a line map from it, so your code gets the same highlighting as the built-in programs.

nothing in app.js is specific to any program. programs only talk to the page through memory (screen, clock, keyboard) and PRINT, so the visualizer runs anything you can assemble.

### building and running
web version (needs emscripten):
```
emcc machine.c web.c asm_web.c -o web/vm.js "-sEXPORTED_FUNCTIONS=_vm_reset,_vm_load_byte,_vm_step,_vm_reg,_vm_pc,_vm_flags,_vm_halted,_vm_mem,_vm_poke,_vm_run,_web_assemble,_web_byte,_web_map" "-sEXPORTED_RUNTIME_METHODS=ccall,cwrap"
emrun web/index.html
```
it has to be served over http (not opened as a file) because the page fetches the program files.

terminal vm and assembler (needs a C compiler):
```
gcc main.c machine.c -o vm
gcc asm.c -o asm
./asm program.asm program.bin
./vm program.bin
```

adding a program to the page: assemble it into web/progs/, copy the .asm next to it, and add it to the picker in index.html:
```
<option value="progs/myprog.bin" data-speed="40" data-hint="what to do">my program</option>
```
data-speed is the slider position (0 to 100) it loads with; data-hint shows up in the i/o box.

### tests
tests/ has small programs with their expected output (.asm + .expected) for arithmetic, control flow and memory. run them all with:
```
tests/run_tests.ps1
```

### project layout
```
vm.h            shared definitions: opcodes, flags, the machine struct
machine.c       the vm: fetch, decode, execute, step, run
main.c          terminal front end
asm.c           the assembler (command line)
asm_web.c       the same assembler, for the browser
web.c           bridge between the vm and the page
tests/          test programs and expected outputs
web/
  index.html    the page
  app.js        run loop, every block, wires, custom mode
  disasm.js     turns the next instruction back into readable text
  style.css     the look
  vm.js, vm.wasm  the compiled vm + assembler
  progs/        programs: .asm, .bin, .bin.map
```

### credits
- i used claude to generate pip's drawing (pip.asm) through a script from my own sketch.
- the random number constants (25173, 13849) are a classic linear congruential generator. RGB565 is a standard color format. the font is jetbrains mono.
- flappy bird and langton's ant are well-known ideas; these versions are written from scratch in pip assembly.