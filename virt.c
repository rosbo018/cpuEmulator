#include "virt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/**
sample program:
00:84 53
03:60
53:54
54:03

 **/
static unsigned char ram[256];

unsigned char ac;
unsigned char ar;
unsigned char dr;
unsigned char ir;
static unsigned char iter;

void (*instructions[0xA1])();

//memory reference
void AND_D(unsigned char mem){
  dr = ram[mem];
  ac = ac & dr;
 iter++;
}

void ADD_D(unsigned char mem){
  dr = ram[mem];
  ac += dr;
 iter++;
}

void SUB_D(unsigned char mem){
  dr = ram[mem];
  ac -= dr;
 iter++;
}

void LDA_D(unsigned char mem){
  dr = ram[mem];
  //printf("%x", dr);
  ac = dr;
  iter++;
}

void STA_D(unsigned char mem){
 ram[mem] = ac;
 iter++;
}

void BUN_D(unsigned char mem){
  //printf("BUN\n");
  iter = mem;
  //printf("%x\n", iter);
}

void ISZ_D(unsigned char mem){
  ram[mem]++;
  if(ram[mem] == 0)
    iter+=3;
  else
    iter++;
}
// indirect
void IND_MREF(unsigned char inst, unsigned char indmem){
  //printf("indirect: %x, %x", inst-0x80 ,ram[indmem] );
    fflush(stdout);
  (*instructions[inst-0x80])(ram[indmem]);
}


//register reference
void CLA(){
  ac = 0;
}
void CMA(){
  ac = ~ac;
}
void ASL(){
  ac = ac << 1;
}
void ASR(){
  ac = ac >> 1;
}
void INC(){
  ac++;
}
//halt is implicit in the interpreitation
void init(){
  instructions[0x01] = &AND_D;
  instructions[0x02] = &ADD_D;
  instructions[0x03] = &SUB_D;
  instructions[0x04] = &LDA_D;
  instructions[0x08] = &STA_D;
  instructions[0x10] = &BUN_D;
  instructions[0x20] = &ISZ_D;

  instructions[0x41] = &CLA;
  instructions[0x42] = &CMA;
  instructions[0x44] = &ASL;
  instructions[0x48] = &ASR;
  instructions[0x50] = &INC;
}

void fileInput( char *fileInputName){
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  FILE *assembly = fopen(fileInputName, "r");
  if (assembly == NULL)
    exit(EXIT_FAILURE);
  while((read = getline(&line, &len, assembly)) != -1){
    printf("%s", line);
    fflush(stdout);
    if(len >= 2)
      assemblyHandle(line);
  }
  free(line);
  
}
unsigned char parse_char(char c){
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return 10 + c - 'a';
    if ('A' <= c && c <= 'F') return 10 + c - 'A';
    printf("UNKNOWN INPUT");
    fflush(stdout);
    exit(1);
}
unsigned char parseHex(char *num){
  //printf("parseing '%s'\n", num);
  return ((parse_char(*(num)) * 0x10) + parse_char(*(num+1)));
}

//
void assemblyHandle(char *line){
    char *rest = line;

    char *loc;
    unsigned char c_inst;
    unsigned char c_loc;
    unsigned char c_memloc;
    loc = strtok_r(rest, ":", &rest); 
    c_loc = parseHex(loc);
    loc = strtok_r(rest, " ", &rest); 
    c_inst = parseHex(loc);
    *(ram+((int)c_loc)) = c_inst;
    if((loc = strtok_r(rest, " ", &rest)) != NULL){
        c_memloc = parseHex(loc);
        *(ram+((int)c_loc)+1) = c_memloc;
    }

}

void run(unsigned char startLocation){
  iter = startLocation;
  
  while((ir = ram[iter++]) != 0x60){
    //char c = getc(stdin);
    //printf("iter %d\n", iter-1);
    if(ir == 0x0)
      iter++;
    if(0x01 <= ir && ir <= 0x20){
      // printf("op: %x; mem: %x\n", ir, ram[iter]);
      (*instructions[ir])(ram[iter]);
    }
    else if((0x81 <= ir && ir <= 0xA0)){
      //printf("op: %x; mem: %x\n", ir, ram[iter]);
      IND_MREF(ir, ram[iter]);
    }
    else if((0x41 <= ir && ir <= 0x60)){
      //printf("op: %x\n",  ir);
      (*instructions[ir])();
    }
    else{
      printf("unrecognized instruction: %x\n", ir);
      fflush(stdout);
      exit(1);
    }
    //printf("\n%x\n", ac);

    }
  printf("\n%x\n", ac);
}

int main(int argc, char *argv[]){
  init();
  //printf("%s\n\n", argv[1]);
  fileInput(argv[1]);
  if(argv[2] != NULL)
    run(atoi(argv[2]));
  else
    for(int i = 0; i < 0xFF; i++)
      if(ram[i] != 0)
        printf("%d: %x\n",i, ram[i]);
  return 0;
}