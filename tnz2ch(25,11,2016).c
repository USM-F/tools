// ���-90  - ������� ���������
// ��������� ��� ������� ��������� (��������) 2 ���� 
// ����������� �������� ������ �� ���, �������� 150 ����
// 12 MHz, 28.09.2016 
// ��� ������������
// ��� �������, ������ � ���������


#include <16f876.h>
#include <delay14.h>
#include <int16c4k.h>

//_________________________________________________________
#define		VERSION		0x01		// ������ ��

#define 	TRUE		1
#define		FALSE		0

#define 	ON		1
#define		OFF		0

#define		BSI_BUF_SIZE	6		// 12 ���� ���� �� ����� 14 ��� ��� ����� � ��� �������

#define		MARKER				PORTB.7		

/*
#define		COMPRESSION_STRAIGHT_IMP	PORTC.0		// �������� ������� ������
#define		COMPRESSION_INVERSE_IMP		PORTC.1

#define		TENSION_STRAIGHT_IMP		PORTC.2		// �������� ������� ����������
#define		TENSION_INVERSE_IMP		PORTC.3
*/

//08.11.2016
#define		COMPRESSION_STRAIGHT_IMP	PORTC.3		// �������� ������� ������
#define		COMPRESSION_INVERSE_IMP		PORTC.2

#define		TENSION_STRAIGHT_IMP		PORTC.0		// �������� ������� ����������
#define		TENSION_INVERSE_IMP		PORTC.1


#define		MARKER_WORK			PORTC.4	

#define		ErrorReadByteUsart 		Flags.0

#define		Unic_N_H			0x01		// ������� ���� ����������� ������ �������
#define		Unic_N_L			0x02		// * ������� ���� ����������� ������ �������
#define 	ToolCode        		0xC8    	// ����� �������


#define 	NUM_OF_AVERAGES       	 	4	    	// ���������� �� NUM ���������� 

//_________________________________________________________

// �������� �������� � ���������
bits		Flags;

unsigned char	Clear_RCREG;
unsigned char	Address;		// ������� ���� �������
unsigned char	Command;		// ������� ���� �������
unsigned char	CheckSum;		// ����������� �����
unsigned char	Var;
unsigned char	PULSE;
unsigned char	i;
unsigned char	CNT_IMP;		//������� ���������, �������� 1..4

unsigned long	BSI_BUF[BSI_BUF_SIZE];
unsigned long	PREVIOUS_DATA[BSI_BUF_SIZE];
unsigned long	DATA_ADC;
unsigned long   MAX_VALUE_ADC;
unsigned long   TEMP1;
unsigned long   TEMP2;
unsigned long   TEMP;

void init();				// �������������
void ReadADC();				// ���������
void ReadByteUsart();                   // ����� ����� ���������� �� USART 
void CRC();				// ������� ����������� �����
void TRANSMISSION();			// ��������
void Test();				// ���� ����� �����
//_________________________________________________________
__INT()
{	
save_context;
	if (PIR1.TMR1IF)
	{
		PIR1.TMR1IF=OFF;

		if ((TENSION_STRAIGHT_IMP==OFF) && (TENSION_INVERSE_IMP==OFF) && (COMPRESSION_STRAIGHT_IMP==OFF) && (COMPRESSION_INVERSE_IMP==OFF)) 		
		{
			if (CNT_IMP>4)	{CNT_IMP = 1;}
			if (CNT_IMP==1)	{TENSION_STRAIGHT_IMP = ON;}
			if (CNT_IMP==2)	{COMPRESSION_STRAIGHT_IMP = ON;}
			if (CNT_IMP==3)	{TENSION_INVERSE_IMP = ON;}
			if (CNT_IMP==4)	{COMPRESSION_INVERSE_IMP = ON;}
			CNT_IMP++;
			TMR1H = 0xFD; TMR1L = 0x11; 		// 2 ����
			//TMR1H = 0xFE; TMR1L = 0x88; 		// 1 ����

		}
		else
		{
			TENSION_STRAIGHT_IMP = OFF;	COMPRESSION_STRAIGHT_IMP = OFF;
			TENSION_INVERSE_IMP = OFF;	COMPRESSION_INVERSE_IMP = OFF;
			//TMR1H = 0xFA; TMR1L = 0x23;			// 4 ����
			TMR1H = 0xF4; TMR1L = 0x47;			// 8 ����
		}		
	}	
restore_context;
}
//_________________________________________________________
void init()
{
	// ���� � ���������� 
	TRISB.0 = OFF; TRISB.1 = OFF; TRISB.7 = OFF;
	PORTB = OFF; 

	TRISC = 0xA0;		// TX - �����, Tension_imp_1, 2 � Compression_imp_1,2 - �������� (������),  RX - ����, ��������� �� ������������
	PORTC.6 = ON; 

	TENSION_STRAIGHT_IMP = OFF;	TENSION_INVERSE_IMP = OFF; 
	COMPRESSION_STRAIGHT_IMP = OFF;	TENSION_INVERSE_IMP = OFF;


// ��������� ����������������� ����� 
        TXSTA = 0x26;		// 8 ���, �������� ���������, ������� ��������
        RCSTA = 0x90;		// 8 ���, ����� ��������
        SPBRG = 4;		// 150 ����	
// ��������� ���
	ADCON1.ADFM = 1;	// ������������ ������ ���������� �����
	ADCON0 = 0x80;		// ������� 10 MHz (����� ��������� 1 ���� - 3.2 �����, 10 ��� - 64 �����)
				// ������ ����� A0			 
// �������
	T1CON = 0x30;		// ������ �������� � �������� 1:8 (3.2 �����)
	PIE1.TMR1IE = ON;	// ���������� �� ������������ ������� 1 ���������
	PIR1.TMR1IF = OFF;
	TMR1H = 0xFF; TMR1L = 0xFF;
	T1CON.TMR1ON = ON;	// �������� ������ 1
	INTCON.PEIE = ON;	// ������������ ���������� ���������
	T2CON = 0x7B;		// ������ 2 �������� 1:256 (102.4 �����)
	Flags = OFF;
// ����������
	CNT_IMP = 0; TEMP1 = 0;	
	MARKER_WORK = OFF; MARKER = OFF;
}
//_________________________________________________________
void ReadADC()
{
	Delay_Us_20MHz(10); while(ADCON0.GO); 	
	DATA_ADC = ADRESH; DATA_ADC<<= 8; DATA_ADC+= ADRESL; 
	MAX_VALUE_ADC = DATA_ADC;

	MARKER = OFF;
	for (i=0; i<30; i++) {
	MARKER = ON; ADCON0.GO = ON; 	  			// ��������� ��������������

	Delay_Us_20MHz(10); while(ADCON0.GO); 
	DATA_ADC = ADRESH; DATA_ADC<<=8; DATA_ADC+=ADRESL;
 
	if (DATA_ADC>MAX_VALUE_ADC) MAX_VALUE_ADC = DATA_ADC;
        MARKER = OFF;			
			     }

}
//_________________________________________________________
void ReadByteUsart()
{
          ErrorReadByteUsart=0;
	  TMR2 = 0; PR2=0x27;        		// �� ������������ 4 ����
          PIR1.TMR2IF = OFF;
	  T2CON.TMR2ON = ON;			// �������� ������ 2
loopTime: if(PIR1.TMR2IF==0) 
          {
          	if(PIR1.RCIF) goto end_Read;
                goto loopTime;
          }   
          ErrorReadByteUsart=1;
end_Read: T2CON.TMR2ON = OFF;			// ��������� ������ 2
}
//_________________________________________________________
void CRC()
{
     	CheckSum^=Var;
     	if (CheckSum&1){ CheckSum>>=1; CheckSum|=0x80; }
     	else { CheckSum>>=1; CheckSum&=0x7F; }
     	if ( CheckSum>127 ) CheckSum^=0x3C; 
}
//_________________________________________________________

void main()
{
	init(); INTCON.GIE = ON;

work:   if (!PIR1.RCIF) goto work;

	if (RCSTA.OERR) { Clear_RCREG=RCREG; Clear_RCREG=RCREG; Clear_RCREG=RCREG; RCSTA.CREN=0; RCSTA.CREN=1; }	//���� ������ ������� ����������� ������ ��� �������

	Address = RCREG; ReadByteUsart(); Command=RCREG;

	if ((Address & 0xF8) != ToolCode ) goto work;
	if (ErrorReadByteUsart) goto work; 

	if ((Address & 7)==1)					// �������� ������, � ����� ���������
	{

		TRANSMISSION();					// �������� ������ ����������� ���������

// ��������� ������
		ADCON0=0x91;	Delay_Us_20MHz(14);		// ��� ���

		TEMP = BSI_BUF[1];
		ADCON0.CHS2 = 0; ADCON0.CHS1 = 0; ADCON0.CHS0 = 0;	// ������� ����� (���������� ������)
		while(TENSION_STRAIGHT_IMP); while(TENSION_STRAIGHT_IMP==0);
		MARKER = ON; ADCON0.GO = ON; ReadADC();
		TEMP1 = TEMP + MAX_VALUE_ADC + MAX_VALUE_ADC + MAX_VALUE_ADC; TEMP = TEMP1>>2;			//����� ������ 75% �� ���������
		BSI_BUF[1] = TEMP;

		TEMP = BSI_BUF[4];
		ADCON0.CHS2 = 0; ADCON0.CHS1 = 1; ADCON0.CHS0 = 1;	// ������ ����� (������ ������)
		while(COMPRESSION_STRAIGHT_IMP); while(COMPRESSION_STRAIGHT_IMP==0);
		MARKER = ON; ADCON0.GO = ON; ReadADC();
		TEMP1 = TEMP + MAX_VALUE_ADC + MAX_VALUE_ADC + MAX_VALUE_ADC; TEMP = TEMP1>>2;
		BSI_BUF[4] = TEMP;

		TEMP = BSI_BUF[2];
		ADCON0.CHS2 = 0; ADCON0.CHS1 = 0; ADCON0.CHS0 = 1;	// ������ ����� (���������� ������)
		while(TENSION_INVERSE_IMP); while(TENSION_INVERSE_IMP==0);
		MARKER = ON; ADCON0.GO = ON; ReadADC();
		TEMP1 = TEMP + MAX_VALUE_ADC + MAX_VALUE_ADC + MAX_VALUE_ADC; TEMP = TEMP1>>2;
		BSI_BUF[2] = TEMP;

		TEMP = BSI_BUF[5];
		ADCON0.CHS2 = 1; ADCON0.CHS1 = 0; ADCON0.CHS0 = 0;	// ��������� ����� (������ ������)
		while(COMPRESSION_INVERSE_IMP); while(COMPRESSION_INVERSE_IMP==0);
		MARKER = ON; ADCON0.GO = ON; ReadADC();
		TEMP1 = TEMP + MAX_VALUE_ADC + MAX_VALUE_ADC + MAX_VALUE_ADC; TEMP = TEMP1>>2;
		BSI_BUF[5] = TEMP;

		TEMP = BSI_BUF[3];
		ADCON0.CHS2 = 0; ADCON0.CHS1 = 1; ADCON0.CHS0 = 0;	// ������ ����� (����������)
		MARKER = ON; ADCON0.GO = ON; ReadADC();
		TEMP1 = TEMP + TEMP +TEMP + MAX_VALUE_ADC; TEMP = TEMP1>>2;					//����� ������ 25% �� ���������				.
		BSI_BUF[3] = TEMP;

		ADCON0.ADON = OFF; 				// ��������� ���

	}
	if ((Address & 7)==5) Test(); 
	goto work;
}
//__________________����� main()__________________________________

void TRANSMISSION()			// ��������
{
Delay_Us_20MHz(250); Delay_Us_20MHz(250);	// ��� ���������� ����� �����
/*
������������ ���� ������
0 - ����� �������, ����
1 - ���������� ������ �����
2 - ���������� ������ �����
3 - ����������
4 - ������ ������ �����
5 - ������ ������ �����
6 - ����������� ����� � ��� �������
*/
MARKER_WORK = ON; 

BSI_BUF[0] = BSI_BUF_SIZE + BSI_BUF_SIZE;	//�������� ���� �������� �� AMSX, ������ ���� ��� -4
CheckSum=0;

// �������� ������
	for (i=0; i<BSI_BUF_SIZE; i++)	//�������� ���� �������� �� AMSX, ������ ���� ��� -2
		{
		Var = BSI_BUF[i];	TXREG = Var; CRC(); while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
		Var = BSI_BUF[i]>>8;	TXREG = Var; CRC(); while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
		}

		TXREG = CheckSum;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24); // ����������� �����
		TXREG = ToolCode;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24); // ����� �������

MARKER_WORK = OFF;
}
//_________________________________________________________
void Test()
{
        Delay_Us_20MHz(250);	Delay_Us_20MHz(250); 		// ��� ���������� ����� �����	

        TXREG=20;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
	TXREG=0;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
	TXREG=0xAA;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
	TXREG=0xC0;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=0x55;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=0x3F;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=0x31;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=0xF7;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=0;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=0x80;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=0;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=0;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=0xFF;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=0xFF;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=0xCE;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=0x08;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=ToolCode;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=ToolCode;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24); 
        TXREG=Unic_N_L;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=Unic_N_H;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=VERSION;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);
        TXREG=0;	while(TXSTA.TRMT==0); Delay_Us_20MHz(24);    
}
//__________________�����__________________________________


