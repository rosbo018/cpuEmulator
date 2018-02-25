# cpuEmulator
an emulator for the cpu that was made in class
This emulator is based on a simple single accumulator system. Thus the emulator is very simple, and the type of assembly that one can write for it is also very simple.

Main files are virt.c and virt.h. the files 72, test are hand assembled files that can be run with the system by inputting their file names as the first argument and inputting 0 as the second argument.

I'm planning on writing an assembler for this because as it stands right now, one has to assemble the assembly by hand. Which also means keeping track of the ram locations for each opcode (not a very nice idea).
