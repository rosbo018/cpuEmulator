#ifndef COMPUTERWARS_H
#define COMPUTERWARS_H



void CLA();
void CMA();
void ASL();
void ASR();

void AND_D(unsigned char );
void ADD_D(unsigned char );
void SUB_D(unsigned char );
void LDA_D(unsigned char );
void STA_D(unsigned char );
void BUN_D(unsigned char );
void ISZ_D(unsigned char );
void fileInput( char *);
void assemblyHandle(char *);
unsigned char parseHex(char *);
unsigned char parse_char(char);

void IND_MREF(unsigned char, unsigned char);

#endif//COMPUTERWARS_H
