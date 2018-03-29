#ifndef __usart_software_H
#define __usart_software_H


unsigned char		N;
unsigned char		ErrorReadByteUsart;
unsigned char   	RCREGSL, TXREGSL;	//для блока датчика
unsigned char		RCREGAC, TXREGAC;	//для блока акселерометров

int ReadByteUsartSL(void); //прием по usart данных натяжения на 150 кбод - программно 
void WriteByteUsartSL(void);//передача по usart данных натяжения на 150 кбод - программно  

int ReadByteUsartAC(void); //прием по usart данных акселерометра на 150 кбод - программно 
void WriteByteUsartAC(void);//передача по usart данных акселерометра на 150 кбод - программно  

void Delay_OneCycle(unsigned char NUM);	//задержка кратностью 1 цикл

#endif