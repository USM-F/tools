#ifndef __usart_software_H
#define __usart_software_H


unsigned char		N;
unsigned char		ErrorReadByteUsart;
unsigned char   	RCREGSL, TXREGSL;	//��� ����� �������
unsigned char		RCREGAC, TXREGAC;	//��� ����� ��������������

int ReadByteUsartSL(void); //����� �� usart ������ ��������� �� 150 ���� - ���������� 
void WriteByteUsartSL(void);//�������� �� usart ������ ��������� �� 150 ���� - ����������  

int ReadByteUsartAC(void); //����� �� usart ������ ������������� �� 150 ���� - ���������� 
void WriteByteUsartAC(void);//�������� �� usart ������ ������������� �� 150 ���� - ����������  

void Delay_OneCycle(unsigned char NUM);	//�������� ���������� 1 ����

#endif