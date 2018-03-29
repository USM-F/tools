#ifndef __M2_H
#define __M2_H

#include <definitions.h>

//extern unsigned char	Rd1;
//extern unsigned char	Rd2;
extern unsigned char	R1;
extern unsigned char	R2;
extern unsigned char	SignalH;
extern unsigned char	SignalL;

extern unsigned int		COMMAND;
extern unsigned int		COMMAND_Before;
//extern unsigned int		Code;
extern unsigned int		DATA;
extern unsigned int		counter;		// счетчик записанных байт

void Transmit_Low(unsigned int	inCode);	// передача 1-го слова M2
void Recieve_Command_M2(void);	//принять команду с поверхности
void Read_Bit(void);	//прием бита по манчестеру
void Check_CRC(void);

void Read_ID(void);					// прочитать ID из flash
void Read_CAL(void);				// прочитать CAL из flash
void Write_Byte_ID_CAL(void);		// прочитать CAL или flash
void TEST(void);					// тест линии связи


#endif


