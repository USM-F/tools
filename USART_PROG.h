#ifndef __USART_PROG_H
#define __USART_PROG_H

#include <definitions.h>

extern unsigned char		ErrorReadByteUsart;

void GET_ACS(void);			//сбор данных от Ax (D710)
void GET_TNZ(void);			//сбор данных от платы TNZ (C910)

int ReadByteUsartSL(void); //прием по usart данных натяжения на 150 кбод - программно 
void WriteByteUsartSL(void);//передача по usart данных натяжения на 150 кбод - программно  

int ReadByteUsartAC(void); //прием по usart данных акселерометра на 150 кбод - программно 
void WriteByteUsartAC(void);//передача по usart данных акселерометра на 150 кбод - программно  

#endif



