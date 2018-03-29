/*
������ ��� ������ � ������� �������� � ��������������
����� ����������� usart
24.02.2015
*/

#include <USART_PROG.h>

#define six_mcs() {_asm nop nop	nop nop nop nop nop nop nop nop nop nop	nop nop nop nop nop nop _endasm}	//6 mcs

unsigned char		USART_BIT;
unsigned char		ErrorReadByteUsart;
unsigned char  	 	RCREGSL, TXREGSL;	//��� ����� �������
unsigned char		RCREGAC, TXREGAC;	//��� ����� ��������������
unsigned int		USART_VAR;			//�������
signed int			Return_;			//����������� USART ����������

//.................................................................................

void GET_TNZ(void)				//���� ������ �� ����� TNZ (C910)
{	unsigned int Dec;
	//Bit_Test=Decode;
	Bit_Test = Dec = 0xC910;

	TXREGSL = Bit_Test>>8; WriteByteUsartSL();	//������ ������� � ����� TNZ
	six_mcs();	//6 mcs
	TXREGSL=Dec; WriteByteUsartSL();
//timer_
	TMR0H = 0xE2; TMR0L = 0xB3;	//5ms �� ������������
	T0CON = 0b10000000;	INTCONbits.TMR0IF = OFF;	//set up timer0 - prescaler 1:2, 16-bit timer, timer ON (7 bit T0CON)
//timer__
	while(RXSL) {if (INTCONbits.TMR0IF)	{goto BREAK_LOOP_TNZ;}};				//���� ������ �� ���������� TNZ			
Return_ = ReadByteUsartSL(); USART_BIT=RCREGSL;
	while(RXSL) {if (INTCONbits.TMR0IF)	{goto BREAK_LOOP_TNZ;}};		
Return_ = ReadByteUsartSL(); USART_BIT=RCREGSL;		//���������� ����� �������� ������

for (USART_VAR=Sstart;USART_VAR<=Eend;USART_VAR++) 
					{
					while(RXSL) {if (INTCONbits.TMR0IF)	{goto BREAK_LOOP_TNZ;}}; 
					Return_ = ReadByteUsartSL(); BSI_BUF[USART_VAR] = RCREGSL;
					}	//����� 5 �������� ����

BREAK_LOOP_TNZ: INTCONbits.TMR0IF = OFF; T0CONbits.TMR0ON = OFF;
}
//....................................................................................
void GET_ACS(void)				//���� ������ �� Ax (D710)
{	unsigned int Dec;
	//Bit_Test=Decode;
	Bit_Test = Dec = 0xD910;

	TXREGAC=Bit_Test>>8; WriteByteUsartAC();	//������ ������� � ����� TNZ
	six_mcs();	//6 mcs
	TXREGAC=Dec; WriteByteUsartAC();
//timer_
	TMR0H = 0xD6; TMR0L = 0xD8;	//7ms �� ������������
	T0CON = 0b10000000;	INTCONbits.TMR0IF = OFF;	//set up timer0 - prescaler 1:2, 16-bit timer, timer ON (7 bit T0CON)
//timer__
	while(RXAC) {if (INTCONbits.TMR0IF)	{goto BREAK_LOOP_ACS;}};				//���� ������ �� ���������� ACS			
Return_ = 	ReadByteUsartAC(); USART_BIT=RCREGAC;
	while(RXAC) {if (INTCONbits.TMR0IF)	{goto BREAK_LOOP_ACS;}};		
Return_ = 	ReadByteUsartAC(); USART_BIT=RCREGAC;		//���������� ����� �������� ������

for (USART_VAR=Sstart;USART_VAR<=Eend;USART_VAR++) 
					{
					while(RXAC) {if (INTCONbits.TMR0IF)	{goto BREAK_LOOP_ACS;}};
					Return_ = ReadByteUsartAC(); BSI_BUF[USART_VAR] = RCREGAC;
					}	//����� 19 �������� ����

BREAK_LOOP_ACS: INTCONbits.TMR0IF = OFF; T0CONbits.TMR0ON = OFF;
}
//============================================================================================================================


//��������� ���� ������ �� TNZ
int ReadByteUsartSL(void)	//��������� ���� ������ �� TNZ  	
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
void WriteByteUsartSL(void)            // �������� 150 ���� (1 ��� 8), ������� ���� ������ � TNZ
{
// ����� ���
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
			NOP(); NOP(); NOP(); NOP(); //��� 1.333 
}


//....................................................................................
int ReadByteUsartAC(void)		//��������� ���� ������ �� AC
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
void WriteByteUsartAC(void)            // �������� 150 ���� (1 ��� 8), ������� ���� ������ � ACS
{
// ����� ���
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
        	NOP(); NOP(); NOP(); NOP(); //��� 1.333
}
