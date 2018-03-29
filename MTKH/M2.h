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
extern unsigned int		counter;		// ������� ���������� ����

void Transmit_Low(unsigned int	inCode);	// �������� 1-�� ����� M2
void Recieve_Command_M2(void);	//������� ������� � �����������
void Read_Bit(void);	//����� ���� �� ����������
void Check_CRC(void);

void Read_ID(void);					// ��������� ID �� flash
void Read_CAL(void);				// ��������� CAL �� flash
void Write_Byte_ID_CAL(void);		// ��������� CAL ��� flash
void TEST(void);					// ���� ����� �����


#endif


