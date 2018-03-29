#ifndef __USART_PROG_H
#define __USART_PROG_H

#include <definitions.h>

extern unsigned char		ErrorReadByteUsart;

void GET_ACS(void);			//���� ������ �� Ax (D710)
void GET_TNZ(void);			//���� ������ �� ����� TNZ (C910)

int ReadByteUsartSL(void); //����� �� usart ������ ��������� �� 150 ���� - ���������� 
void WriteByteUsartSL(void);//�������� �� usart ������ ��������� �� 150 ���� - ����������  

int ReadByteUsartAC(void); //����� �� usart ������ ������������� �� 150 ���� - ���������� 
void WriteByteUsartAC(void);//�������� �� usart ������ ������������� �� 150 ���� - ����������  

#endif



