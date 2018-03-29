/*
модуль для работы с блоками датчиков и акселерометров
через программный usart
24.02.2015
*/

#include <USART_PROG.h>

#define six_mcs() {_asm nop nop	nop nop nop nop nop nop nop nop nop nop	nop nop nop nop nop nop _endasm}	//6 mcs

unsigned char		USART_BIT;
unsigned char		ErrorReadByteUsart;
unsigned char  	 	RCREGSL, TXREGSL;	//для блока датчика
unsigned char		RCREGAC, TXREGAC;	//для блока акселерометров
unsigned int		USART_VAR;			//счетчик
signed int			Return_;			//Программный USART возвращает

//.................................................................................

void GET_TNZ(void)				//сбор данных от платы TNZ (C910)
{	unsigned int Dec;
	//Bit_Test=Decode;
	Bit_Test = Dec = 0xC910;

	TXREGSL = Bit_Test>>8; WriteByteUsartSL();	//подача команды в плату TNZ
	six_mcs();	//6 mcs
	TXREGSL=Dec; WriteByteUsartSL();
//timer_
	TMR0H = 0xE2; TMR0L = 0xB3;	//5ms до срабатывания
	T0CON = 0b10000000;	INTCONbits.TMR0IF = OFF;	//set up timer0 - prescaler 1:2, 16-bit timer, timer ON (7 bit T0CON)
//timer__
	while(RXSL) {if (INTCONbits.TMR0IF)	{goto BREAK_LOOP_TNZ;}};				//ввод данных от процессора TNZ			
Return_ = ReadByteUsartSL(); USART_BIT=RCREGSL;
	while(RXSL) {if (INTCONbits.TMR0IF)	{goto BREAK_LOOP_TNZ;}};		
Return_ = ReadByteUsartSL(); USART_BIT=RCREGSL;		//вычитываем длину полезных данных

for (USART_VAR=Sstart;USART_VAR<=Eend;USART_VAR++) 
					{
					while(RXSL) {if (INTCONbits.TMR0IF)	{goto BREAK_LOOP_TNZ;}}; 
					Return_ = ReadByteUsartSL(); BSI_BUF[USART_VAR] = RCREGSL;
					}	//всего 5 полезных слов

BREAK_LOOP_TNZ: INTCONbits.TMR0IF = OFF; T0CONbits.TMR0ON = OFF;
}
//....................................................................................
void GET_ACS(void)				//сбор данных от Ax (D710)
{	unsigned int Dec;
	//Bit_Test=Decode;
	Bit_Test = Dec = 0xD910;

	TXREGAC=Bit_Test>>8; WriteByteUsartAC();	//подача команды в плату TNZ
	six_mcs();	//6 mcs
	TXREGAC=Dec; WriteByteUsartAC();
//timer_
	TMR0H = 0xD6; TMR0L = 0xD8;	//7ms до срабатывания
	T0CON = 0b10000000;	INTCONbits.TMR0IF = OFF;	//set up timer0 - prescaler 1:2, 16-bit timer, timer ON (7 bit T0CON)
//timer__
	while(RXAC) {if (INTCONbits.TMR0IF)	{goto BREAK_LOOP_ACS;}};				//ввод данных от процессора ACS			
Return_ = 	ReadByteUsartAC(); USART_BIT=RCREGAC;
	while(RXAC) {if (INTCONbits.TMR0IF)	{goto BREAK_LOOP_ACS;}};		
Return_ = 	ReadByteUsartAC(); USART_BIT=RCREGAC;		//вычитываем длину полезных данных

for (USART_VAR=Sstart;USART_VAR<=Eend;USART_VAR++) 
					{
					while(RXAC) {if (INTCONbits.TMR0IF)	{goto BREAK_LOOP_ACS;}};
					Return_ = ReadByteUsartAC(); BSI_BUF[USART_VAR] = RCREGAC;
					}	//всего 19 полезных слов

BREAK_LOOP_ACS: INTCONbits.TMR0IF = OFF; T0CONbits.TMR0ON = OFF;
}
//============================================================================================================================


//прочитать байт данных от TNZ
int ReadByteUsartSL(void)	//прочитать байт данных от TNZ  	
{
        RCREGSL=0;
        ErrorReadByteUsart=0;
        // START BIT
        if(RXSL) {ErrorReadByteUsart=1;return -1;}  
        NOP(); NOP(); NOP(); NOP(); NOP(); 
        //NOP(); NOP(); NOP(); NOP(); 
        USART_BIT=8; 

Loop2:  RCREGSL>>=1;
		if(RXSL) {RCREGSL |= 0x80;}
		NOP(); NOP(); NOP(); NOP(); NOP();
        NOP(); NOP(); NOP(); NOP(); NOP();
        NOP(); //NOP(); NOP(); 
_asm         
        decfsz USART_BIT, 1, 1
        goto Loop2
_endasm
        // STOP BIT
		NOP(); NOP(); NOP(); NOP(); NOP();
		NOP();

        if (RXSL) return -2;
        ErrorReadByteUsart=1; 
}
//..................................................................................
void WriteByteUsartSL(void)            // скорость 150 кбод (1 бит 8), послать байт данных в TNZ
{
// старт бит
        TXSL = 0; 
		NOP(); NOP(); NOP(); NOP(); NOP();
        NOP(); NOP(); NOP(); NOP(); NOP();
        NOP(); NOP(); NOP(); 
        USART_BIT=8; 
cmp:    if(TXREGSL&1)goto Wr1;     
        NOP();

Wr0:    TXSL=0; 
        NOP(); NOP(); NOP(); NOP(); NOP();
        NOP(); NOP(); NOP(); NOP(); NOP();
        TXREGSL>>=1;

	_asm
        decfsz USART_BIT, 1, 1
        goto cmp
        goto stopbit
	_endasm

Wr1:    TXSL=1; 
        NOP(); NOP(); NOP(); NOP(); NOP(); 
        NOP(); NOP(); NOP(); NOP(); NOP();
        TXREGSL>>=1;

	_asm
        decfsz USART_BIT, 1, 1
        goto cmp
        goto stopbit
	_endasm

stopbit: 	NOP(); NOP(); 
			TXSL = 1;
			Delay10TCYx(2);		//6.7 mcs
			NOP(); NOP(); NOP(); NOP(); //еще 1.333 
}


//....................................................................................
int ReadByteUsartAC(void)		//прочитать байт данных от AC
{
        RCREGAC=0;
        ErrorReadByteUsart=0;
        // START BIT
        if(RXAC) {ErrorReadByteUsart=1;return -1;}  
		NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); 
		//NOP(); NOP(); NOP(); NOP(); 
        USART_BIT=8; 
Loop3:  RCREGAC>>=1;
		if(RXAC) {RCREGAC|= 0x80;}
		NOP(); NOP(); NOP(); NOP(); NOP();
    	NOP(); NOP(); NOP(); NOP(); NOP();
        NOP();	// NOP(); NOP(); 

	_asm         
        decfsz USART_BIT, 1, 1
        goto Loop3
	_endasm

        // STOP BIT
		NOP(); NOP(); NOP(); NOP(); NOP();
        NOP(); 

        if(RXAC) return -2;
        ErrorReadByteUsart = 1; 
}
//..................................................................................
void WriteByteUsartAC(void)            // скорость 150 кбод (1 бит 8), послать байт данных в ACS
{
// старт бит
        TXAC = 0; 
        NOP(); NOP(); NOP(); NOP(); NOP();
        NOP(); NOP(); NOP(); NOP(); NOP();
        NOP(); NOP(); NOP(); 
        USART_BIT=8; 
cmp0:   if(TXREGAC&1)goto Wr3;     
        NOP();
Wr2:    TXAC=0; 
        NOP(); NOP(); NOP(); NOP(); NOP();
        NOP(); NOP(); NOP(); NOP(); NOP();
        TXREGAC>>=1;

	_asm
        decfsz USART_BIT, 1, 1
        goto cmp0
        goto stopbit0
	_endasm

Wr3:    TXAC = 1;
        NOP(); NOP(); NOP(); NOP(); NOP();
        NOP(); NOP(); NOP(); NOP(); NOP();
        TXREGAC>>=1;

	_asm
        decfsz USART_BIT, 1, 1
        goto cmp0
        goto stopbit0
	_endasm

stopbit0:	NOP(); NOP();
			TXAC=1;
			Delay10TCYx(2);		//6.6 mcs
        	NOP(); NOP(); NOP(); NOP(); //еще 1.333
}
