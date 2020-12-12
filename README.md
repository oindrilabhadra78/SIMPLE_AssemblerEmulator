# SIMPLE_AssemblerEmulator
Assembler and Emulator for SIMPLE Assembly Language

* Written in C89

### For running:-
gcc asm.c -o asm

gcc emu.c -o emu 

./asm file.asm

./emu [options] file.o

-trace  show instruction trace

-before show memory dump before execution

-after  show memory dump after execution

-isa    ISA
