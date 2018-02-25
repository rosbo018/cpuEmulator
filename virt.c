#include "virt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DIRECT_OP_LOWER 0x01
#define DIRECT_OP_UPPER 0x20
#define INDIRECT_OP_LOWER 0x81
#define INDIRECT_OP_UPPER 0xA0
#define AC_OP_LOWER 0x41
#define AC_OP_UPPER 0x60
#define HALT 0x60
#define MEMSIZE 256
/**
   
this program uses numbers to represent place in ram
so you will have to manually add a prefix of where you want the instructions to go example:
00:04 80
00= place in ram you want the instructions to go, 04 will go in 00, and 80 will go in 01

04= the opcode you want to use
80= the memory location (for memory reference opcode only)

if you mess up the numbers, the program will crash

a more user friendly assembler is in the works

for more information, see the program in the 'mult' file, it multiplies the numbers in memory location a0 and a1 together 

inputs are data file and the place to start the execution, example: ./c data 0

outputs are in hex, so don't be suprised

sample program:
00:84 53
02:60
53:54
54:03

 **/
static unsigned char ram[MEMSIZE];
//all locations are defined as 0 by default

//accumulator
unsigned char ac;
//address register
unsigned char ar;
//direct register (feeds into accumulator)
unsigned char dr;
//instruction register
unsigned char ir;
//pc = program counter
static unsigned char iter;//abstract pc

void (*instructions[0xA1])();

//memory reference
void AND_D(unsigned char mem){
  // logical and
  dr = ram[mem];
  ac = ac & dr;
  iter++;
}

void ADD_D(unsigned char mem){
  //addition
  dr = ram[mem];
  ac += dr;
  iter++;
}

void SUB_D(unsigned char mem){
  // subtract
  dr = ram[mem];
  ac -= dr;
  iter++;
}

void LDA_D(unsigned char mem){
  //load
  dr = ram[mem];
 
  //printf("%x", dr);
  ac = dr;
  iter++;
}

void STA_D(unsigned char mem){
  // store 
  ram[mem] = ac;
  iter++;
}

void BUN_D(unsigned char mem){
  //branch uncondictionally
  //printf("BUN\n");
  iter = mem;
  //printf("%x\n", iter);
}

void ISZ_D(unsigned char mem){
  //increments the location in memory and skips the next memory location if zero
  ram[mem]++;
  if(ram[mem] == 0){
    iter+=2;
    if(!(0x41 <= ram[mem+1] && ram[mem+1] <= 0x60))
      iter++;
  }
  else
    iter++;
}
// indirect
void IND_MREF(unsigned char inst, unsigned char indmem){
  //accesses indirect memory, then calls the op with the memory from the reference
  //printf("indirect: %x, %x", inst-0x80 ,ram[indmem] );
  fflush(stdout);
  (*instructions[inst-0x80])(ram[indmem]);
}


//register reference
void CLA(){
  //clear
  ac = 0;
}
void CMA(){
  //compliment (one's compliment)
  ac = ~ac;
}
void ASL(){
  //arithmetic shift left
  ac = ac << 1;
}
void ASR(){
  //arithmetic shift right
  ac = ac >> 1;
}
void INC(){
  //increment
  ac++;
}

//halt is implicitly checked in the execution
void init(){
  //initialises the instruction function pointer array
  //ignore errors from this block of code
  instructions[0x01] = AND_D;
  instructions[0x02] = ADD_D;
  instructions[0x03] = SUB_D;
  instructions[0x04] = LDA_D;
  instructions[0x08] = STA_D;
  instructions[0x10] = BUN_D;
  instructions[0x20] = ISZ_D;
  //the reason is that the function pointers
  //are being assigned onto a pointer array with
  //a different input. It will work in practice

  instructions[0x41] = CLA;
  instructions[0x42] = CMA;
  instructions[0x44] = ASL;
  instructions[0x48] = ASR;
  instructions[0x50] = INC;
}

void fileInput( char *fileInputName){
  //loads the file, handles the lines and puts the instructions into
  //the correct place in the ram.
  //since this function interacts with the global ram, it does not have to return anything.
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  FILE *assembly = fopen(fileInputName, "r");
  if (assembly == NULL)
    exit(EXIT_FAILURE);
  while((read = getline(&line, &len, assembly)) != -1){
    //printf("%s", line);
    fflush(stdout);
    if(len >= 2)
      assemblyHandle(line);
    free(line);
  }
  fclose(assembly);
  
}
unsigned char parse_char(char c){
  //returns the decimal value of hex
  if ('0' <= c && c <= '9')
    return c - '0';
  if ('a' <= c && c <= 'f') 
    return 10 + c - 'a';
  if ('A' <= c && c <= 'F') 
    return 10 + c - 'A';
  printf("UNKNOWN INPUT");
  fflush(stdout);
  exit(1);
}
unsigned char parseHex(char *num){
  //parse the hex 2 digit number to decimal
  //printf("parseing '%s'\n", num);
  return ((parse_char(*(num)) * 0x10) + parse_char(*(num+1)));
}

//
void assemblyHandle(char *line){
  //parses a line of the input file, and puts it in memory
  char *rest = line;
  char *loc;
  unsigned char c_inst;
  unsigned char c_loc;
  unsigned char c_memloc;
  loc = strtok_r(rest, ":", &rest); 
  c_loc = parseHex(loc);//unsigned char, requires int casting for pointer arithmatic
  loc = strtok_r(rest, " ", &rest); 
  c_inst = parseHex(loc);
  *(ram+((int)c_loc)) = c_inst;
  if((loc = strtok_r(rest, " ", &rest)) != NULL){
    c_memloc = parseHex(loc);
    *(ram+((int)c_loc)+1) = c_memloc;
  }

}

void run(unsigned char startLocation){
  //runs the program from the starting location until when the program ends(or fails). 
  //indirect memory opcodes will first go through IND_MREF to find the value of the ram location
  //that they point to, then call the direct memory opcode

  iter = startLocation; //start location as defined by the second argument
  
  while((ir = ram[iter++]) != HALT){
    //char c = getc(stdin);
    //printf("iter %d\n", iter-1);
    if(DIRECT_OP_LOWER <= ir && ir <= DIRECT_OP_UPPER){// direct 
      // printf("op: %x; mem: %x\n", ir, ram[iter]);
      (*instructions[ir])(ram[iter]);
    }
    else if((INDIRECT_OP_LOWER <= ir && ir <= INDIRECT_OP_UPPER)){
      //printf("op: %x; mem: %x\n", ir, ram[iter]);
      IND_MREF(ir, ram[iter]);
    }
    else if((AC_OP_LOWER <= ir && ir <= AC_OP_UPPER)){
      //printf("op: %x\n",  ir);
      (*instructions[ir])();
    }
    else{
      printf("unrecognized instruction: %d:%x\n", iter, ir);
      fflush(stdout);
      exit(1);
    }
    //printf("\n%x\n", ac);

    }
  //prints the ram
  printf("\n%x\n", ac);
  for(int i = 0; i < 0xFF; i++)
      if(ram[i] != 0)
        printf("%d: %x\n",i, ram[i]);
}

int main(int argc, char *argv[]){
  init();
  //printf("%s\n\n", argv[1]);
  if(argc == 0 || argc > 3){
    printf("inputs are data file and the place\
to start the execution, example: ./c data 0");
    exit(1);
  }
  fileInput(argv[1]);
  if(argv[2] != NULL)
    run(atoi(argv[2]));
  else
    for(int i = 0; i < 0xFF; i++)
      if(ram[i] != 0)
        printf("%d: %x\n",i, ram[i]);
  return 0;
}
