/*
05.09.2016 - ��������� ��� ������������ ������ ��+��+� ���������� ������
���� ��������� �������
*/
//-------------------------------------------------------------------------------------
#pragma config WDTEN = OFF
#pragma config FOSC         =    HS1     //12MHz Crystal, (HS oscillator)
#pragma config PWRTEN = OFF
#pragma config BOREN = OFF				//Brown-out Reset Enable bits (Brown-out Reset disabled in hardware and software)
#pragma config SOSCSEL  = DIG			//RC0, RC1 digital SOSC Power Selection and mode Configuration bits (High Power SOSC circuit selected)
#pragma config XINST  = OFF
//#pragma config PLLCFG  = ON

#include <definitions.h>
//#include <CRC.h>
#include <RS485.h>
#include <USART_PROG.h>

#include <flash.h>

#define	Version	0x0E					//����� ������

#define	Dev		1						//�������� ��� USART 750-0 375-1
#define	Ttime	3						//�������� �� 1 ���� ��� USART � ��� ��� ��
//-----------------

#pragma udata BUFER
unsigned char	BIG_BUF[1152];
#pragma udata

#pragma udata BUFER_BSI
unsigned char	BSI_BUFER[BSI_BUF_SIZE];
#pragma udata

unsigned char	N;				//��� delay_us
unsigned char	i;
unsigned char	GLOBAL_ERROR;	//������ ��� ����� ������ �� ���� ���, ������� ������������ � BUF[3]

unsigned int	START_FLASH_ADDR;

unsigned int	ADRESS_1, ADRESS_2;

unsigned int	Sstart;			//������ ������ �������
unsigned int	Eend;			//��������� ������ �������
unsigned int	cnt;
unsigned int	Bit_Test;
unsigned int	DATA;

unsigned char	SLAVE_CAL_LENGHT;

unsigned char   *BUF;			//� ������� �������� ����� ���������
unsigned char   *BSI_BUF;		//� ������� �������� ����� ��������

union
{
	unsigned int	TYPE_INT;		// ����� ������
	unsigned char	TYPE_CHAR[2];	// ����� ������
} ARRAY_DATA;

struct
{
unsigned char ID		: 1;
unsigned char CAL		: 1;
unsigned char FLASH_COM	: 1;
}	FLAG;

void TNZ_WORK(void);
void ACS_WORK(void);

void Delay_OneCycle(unsigned char NUM);	//�������� ���������� 1 ����
//----------------------------------------------------------------------------
void INIT(void)		//��������� �������������
{

ANCON0 = 0x00;			//��� ������ - ����������
ANCON1 = 0x00;

	TRISA = 0b00011101;	//0 - ����, 1,5 - �����, ��������� �� ������������
	PORTA = OFF;	 
	TXSL = 1;

//---------������������� PORTC � RS-485 EUSART1 ������ � ������������ ���
//	LATCbits.LATC6 = ON; 	//��������� �����, ���� ������� ��� 1 �� usart
//	LATCbits.LATC1 = OFF; 	//��������� �����, ���� ������� ��� 1 �� usart

	TRISC = 0b10010011; PORTC = 0x60;	//0,1 - ����, 4 �� ���., 2,3,5,6  - �����, 6,7- uart, RX (RC7) TX (RC6), ReDe (RC5)

	TXSTA1 = 0x06;		//0000 0110
	RCSTA1 = 0x80;		//1000 0000

//	SPBRG1 = (FOSC + BAUDRATE*8) / (BAUDRATE*16) - 1;
	SPBRG1 = Dev;		//�������� ��� �������� 375 ����

	RCSTA1bits.SPEN = ON;		//�������� ������ USART 
	RCSTA1bits.CREN = OFF;		//����� 2-� ���� ��������
	TXSTA1bits.TXEN = OFF;		//������ ��������
	BAUDCON1bits.BRG16 = OFF;
//---------�������������------------- 

//---------������������� PORTB �  RS-485 EUSART2 ������ � ���������
//	LATBbits.LATB6 = ON; 	//��������� �����, ���� ������� ��� 1 �� usart
//	LATBbits.LATB1 = OFF; 	//��������� �����, ���� ������� ��� 1 �� usart
	TRISB = 0b10010101; PORTB = 0x60; 	//0 - ����, 1, 5 - ����� 4 �� ���., 2,3,5,6  - �����, 6,7- uart RX (RB7) TX (RB6), ReDe (RB5)

	TXAC = 1;

	TXSTA2 = 0x06;		//0000 0110
	RCSTA2 = 0x80;		//1000 0000

//	SPBRG2 = (FOSC + BAUDRATE*8) / (BAUDRATE*16) - 1;
	SPBRG2 = Dev;		//�������� ��� �������� 375 ����

	RCSTA2bits.SPEN = ON;		//�������� ������ USART 
	RCSTA2bits.CREN = OFF;		//����� 2-� ���� ��������
	TXSTA2bits.TXEN = OFF;		//������ ��������
	BAUDCON2bits.BRG16 = OFF;
//---------�������������--------------

	ReDe1 = ON; ReDe2 = OFF;		//�� ����� �������
	ERROR = 0;
	Bit_Test = 0; LENGHT_INT = 0;
	j=2;

BUF = BIG_BUF;	//���������� ���������
BSI_BUF = BSI_BUFER;
	for (j=0;j<BUF_SIZE;j++) {BUF[j] = 0;}
	for (j=0;j<BSI_BUF_SIZE;j++) {BSI_BUF[j] = 0;}
FLAG.CAL = OFF; FLAG.ID = OFF;
	MARKER = ON;
	}
//....................................................................................
void main (void)
{
	INIT();
SIGNAL:	RECIEVE_EUSART2();					//����� �������	
		if ((ADDR_MAIN == BASE_ADDR_LEFT) && (ADDR_ADDITIONAL == BASE_ADDR_RIGHT)) {ADDR_MAIN = ADDR; ADDR_ADDITIONAL = 0x00;}	//����� ����� ��� ����
		if (ADDR_MAIN!= ADDR)		 {goto SIGNAL;}			//�������� ���� ������������ ������

		if (ERROR!= 0)	{if (CMD == ECHO) {TRANSFER_EUSART2(LENGHT_INT);}	else	{TRANSFER_EUSART2(0);} goto SIGNAL;}		//����� �� ������ ������

	switch (ADDR_ADDITIONAL)	{
		case ADDR_BSI_TNZ:		TNZ_WORK();		break;		
		case ADDR_BSI_ACS:		ACS_WORK();		break;	
		case ADDR_BSI_RB:		ARRAY_DATA.TYPE_INT = ADDR_RB;	 BSI_COMMAND_WORK();	break;
		case ADDR_BSI_TD:		ARRAY_DATA.TYPE_INT = ADDR_TD;	 BSI_COMMAND_WORK();	break;

		default:	ADAPTER_COMMAND_WORK();	break;
								}							
	goto SIGNAL;
}
//....................................................................................
void GET_ALL()					//����� ���� ���, ������� 0x00
{
/*
������������������ ������:
1. �������������, ����������� - 38 ���� 
2. ��������� - 6 ����, � ����� �������� 10 ����
3. ������������� - 14 ����
4. ���������-�������� - 8 ����.
*/
//�� ������ ����� �������� ��������� ������ ���� ���� �� 2!!!
for (j=0; j<ALL_DATA_SIZE; j++)	{BUF[j+8] = BSI_BUF[j];}
	if (GLOBAL_ERROR!= 0) {ERROR = INCOMPLETE_DATA_ERROR; GLOBAL_ERROR = 0;}	//������ �� ���������� ���������
//ERROR = 0;	//��� ������� ���� �� ���� C0
		LENGHT_INT = ALL_DATA_SIZE; TRANSFER_EUSART2(LENGHT_INT);			//�������� ����������� ������
		for (j=0;j<ALL_DATA_SIZE;j++) {BSI_BUF[j] = 0;} ERROR = 0;			//�������� ������ �����������

//������������� 38 ����
	MARKER = OFF;	Sstart = 2;			Eend = ACS_DATA_SIZE + 1;	GET_ACS();	MARKER = ON;				//[0],[1] ERROR 2-39 ������� � BSI_BUF 8-47 in BUF
	if (ErrorReadByteUsart) {ERROR = err_code[6];}			//��� ����� � ����������� �����������
	if	(ERROR)	{BSI_BUF[Sstart-1] = ERROR; BSI_BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR; ERROR = 0;}		//�������� ������ �����������

//��������� - ������ 10 ����
	MARKER = OFF;	Sstart = Eend + 3;	Eend = Sstart + TNZ_DATA_SIZE - 1;	GET_TNZ();	MARKER = ON;		//CAL_TNZ_LENGHT	162 ����	 48-59 in BUF
	if (ErrorReadByteUsart) {ERROR = err_code[6];}			//��� ����� � ����������� �����������
	if	(ERROR)	{BSI_BUF[Sstart-1] = ERROR; BSI_BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR; ERROR = 0;}		//�������� ������ �����������

//������������� 14 ����
	ARRAY_DATA.TYPE_INT = ADDR_RB;  CMD = TRANS;	RETRANSFER_EUSART1(2);	RECIEVE_EUSART1();
	Sstart = Eend + 3;	Eend = Sstart + RB_DATA_SIZE - 1;		//7 ���� � ����� ������						//[52],[53] ERROR 54-67 ������� � BSI_BUF 60-75 in BUF
//	for	(j=Sstart;j<=Eend;j+=2) {BSI_BUF[j] = BUF[j-45]; BSI_BUF[j+1] = BUF[j-46];}
	for (j=Sstart;j<=Eend;j++) {BSI_BUF[j] = BUF[j-46];}
	if	(ERROR)	{BSI_BUF[Sstart-1] = ERROR; BSI_BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR;}
//	Delay1KTCYx(6);//����� ����� �������������� 2,6 �� ��� �� ��������

//���������-�������� 8 ����
	ARRAY_DATA.TYPE_INT = ADDR_TD;	 CMD = TRANS;	RETRANSFER_EUSART1(2);	RECIEVE_EUSART1();
	Sstart = Eend + 3;	Eend =  Sstart + TD_DATA_SIZE - 1;		//4 ����� � ����� ������					//[68],[69] ERROR 70-77 ������� � BSI_BUF 76-85 in BUF
//	for	(j=Sstart;j<=Eend;j+=2) {BSI_BUF[j] = BUF[j-59]; BSI_BUF[j+1] = BUF[j-60];}
	for (j=Sstart;j<=Eend;j++) {BSI_BUF[j] = BUF[j-62];}
	if	(ERROR)	{BSI_BUF[Sstart-1] = ERROR; BSI_BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR;}
}
//....................................................................................
void TNZ_WORK(void)
{
switch (CMD)	{
//		case INF:	INF_();		break;
		case ECHO:	TRANSFER_EUSART2(LENGHT_INT);	break;					//���-����, ������� 12 ���� LENGHT_INT = N_BYTES_RECEIVED - 12;
		case TRANS:		for (j=0; j<ALL_DATA_SIZE; j++)	{BUF[j+8] = BSI_BUF[j];}
							if (ERROR == 0)	{
								LENGHT_INT = TNZ_DATA_SIZE; TRANSFER_EUSART2(LENGHT_INT);						//�������� ����������� ������
								for (cnt=Sstart;cnt<=Eend;cnt++) {BSI_BUF[cnt] = 0;}
											} else {TRANSFER_EUSART2(0);}
						MARKER = OFF;	Sstart = 0;	Eend = TNZ_DATA_SIZE - 1;	GET_TNZ();	MARKER = ON;
						if (ErrorReadByteUsart) {ERROR = err_code[6];}									break;	//��� ����� � ����������� �����������	

		case TR_ID:		//��������� �������������
						if	(LENGHT_INT == 0)	{
							if (ADDR_ADDITIONAL == ADDR_BSI_TNZ){					//��������� ��������
							ADRESS_1 = 0x5400;
							ReadFlash((UINT32) ADRESS_1, (UINT16) ID_LENGHT, &BUF[8]);
							LENGHT_INT = (unsigned int) ID_LENGHT;	TRANSFER_EUSART2(LENGHT_INT);
																}
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;

		case ALLOW_ID:												//��������� ������ ��������������
						for (i=8; i<16;i++)	{						//�������� ������
						FLAG.ID = ON;
						if (BUF[i] != Password[i-8]) {FLAG.ID = OFF; ERROR = err_code[12];	break;}
											}	TRANSFER_EUSART2(0);	break; 
		case WR_ID:		//�������� �������������, 128 ����
						if	(LENGHT_INT == ID_LENGHT)	{
							if (FLAG.ID == ON)	{
								LENGHT_INT = (unsigned int) ID_LENGHT; TRANSFER_EUSART2(LENGHT_INT);					//������� ��� �������� �������, ����� ���, � body ����� �����������
								ADRESS_1 = 0x5400;	ADRESS_2 = ADRESS_1 + (UINT32) ID_LENGHT;
								EraseFlash((UINT32) ADRESS_1,(UINT32)ADRESS_2);
								WriteBytesFlash((UINT32)ADRESS_1,(UINT16) ID_LENGHT, &BUF[8]); FLAG.ID = OFF;
								//ReadFlash((UINT32) ADRESS_1, (UINT16) ID_LENGHT, &BUF[8]);							//�����-������������� ��� ������ ����� ������, �� ��� ��� �����
								//LEN = (unsigned char) ID_LENGHT;	reply(LEN);	
												}	else {ERROR = err_code[13]; TRANSFER_EUSART2(0);}					//������ � ������ ���������
														}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;	//����� BODY �� ���������
		case TR_CAL:	//��������� ����������
						if	(LENGHT_INT == 0)	{
							if (ADDR_ADDITIONAL == ADDR_BSI_TNZ)	{//�������� ������ �� ����������� ��������� 19.01.2015
							ADRESS_1 = 0x5500;
							ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_TNZ_LENGHT, &BUF[8]);
							LENGHT_INT = (unsigned int) CAL_TNZ_LENGHT;	TRANSFER_EUSART2(LENGHT_INT);
																	}
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;		//����� BODY �� ���������
		case ALLOW_CAL:	//��������� ������
						if	(LENGHT_INT == 0)	{
											TRANSFER_EUSART2(0); FLAG.CAL = ON;
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;		//����� BODY �� ���������	
		case WR_CAL:	//�������� ����������, 126 ����
					if	(LENGHT_INT == CAL_TNZ_LENGHT)	{
						if (FLAG.CAL == ON)	{
							LENGHT_INT = (unsigned int) CAL_TNZ_LENGHT; TRANSFER_EUSART2(LENGHT_INT);					//����� ���, � body ����� �����������
							ADRESS_1 = 0x5500;	ADRESS_2 = ADRESS_1 + (UINT32) CAL_TNZ_LENGHT;
							EraseFlash((UINT32) ADRESS_1 ,(UINT32) ADRESS_2);
							WriteBytesFlash((UINT32) ADRESS_1,(UINT16) CAL_TNZ_LENGHT, &BUF[8]); FLAG.CAL = OFF; 
							//ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_LENGHT, &BUF[8]);
							//LEN = (unsigned char) CAL_LENGHT;	reply(LEN);											//�����-���������� ��� ������ ����� ������, �� ��� ��� �����
											}	else {ERROR = err_code[13]; TRANSFER_EUSART2(0);}					//������ � ������ ���������
														}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;	//����� BODY �� ���������

		default:	ERROR = err_code[11]; TRANSFER_EUSART2(0); break;	//�������� ��� ������� ����������������
				}	

}		//����� TNZ_WORK(void)
//============================================================================================================================
void ACS_WORK(void)
{
switch (CMD)	{
//		case INF:	INF_();		break;
		case ECHO:	TRANSFER_EUSART2(LENGHT_INT);	break;					//���-����, ������� 12 ���� LENGHT_INT = N_BYTES_RECEIVED - 12;
		case TRANS:		for (j=0; j<ALL_DATA_SIZE; j++)	{BUF[j+8] = BSI_BUF[j];}	//�������� ������
							if (ERROR == 0)	{
							LENGHT_INT = ACS_DATA_SIZE; TRANSFER_EUSART2(LENGHT_INT);	
							for (cnt=Sstart;cnt<=Eend;cnt++) {BSI_BUF[cnt] = 0;}
											} else {TRANSFER_EUSART2(0);}						//�������� ����������� ������
							MARKER = OFF;	Sstart = 0;		Eend = ACS_DATA_SIZE - 1 ;	GET_ACS();	MARKER = ON;
							if (ErrorReadByteUsart) {ERROR = err_code[6];}			break;		//��� ����� � ����������� �����������

		case TR_ID:		//��������� �������������
						if	(LENGHT_INT == 0)	{
							if (ADDR_ADDITIONAL == ADDR_BSI_ACS){					//��������� ��������
							ADRESS_1 = 0x5200;
							ReadFlash((UINT32) ADRESS_1, (UINT16) ID_LENGHT, &BUF[8]);
							LENGHT_INT = (unsigned int) ID_LENGHT;	TRANSFER_EUSART2(LENGHT_INT);
																}
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;

		case ALLOW_ID:												//��������� ������ ��������������
						for (i=8; i<16;i++)	{						//�������� ������
						FLAG.ID = ON;
						if (BUF[i] != Password[i-8]) {FLAG.ID = OFF; ERROR = err_code[12];	break;}
											}	TRANSFER_EUSART2(0);	break; 
		case WR_ID:		//�������� �������������, 128 ����
						if	(LENGHT_INT == ID_LENGHT)	{
							if (FLAG.ID == ON)	{
								LENGHT_INT = (unsigned int) ID_LENGHT; TRANSFER_EUSART2(LENGHT_INT);					//������� ��� �������� �������, ����� ���, � body ����� �����������
								ADRESS_1 = 0x5200;	ADRESS_2 = ADRESS_1 + (UINT32) ID_LENGHT;
								EraseFlash((UINT32) ADRESS_1,(UINT32)ADRESS_2);
								WriteBytesFlash((UINT32)ADRESS_1,(UINT16) ID_LENGHT, &BUF[8]); FLAG.ID = OFF;
								//ReadFlash((UINT32) ADRESS_1, (UINT16) ID_LENGHT, &BUF[8]);							//�����-������������� ��� ������ ����� ������, �� ��� ��� �����
								//LEN = (unsigned char) ID_LENGHT;	reply(LEN);	
												}	else {ERROR = err_code[13]; TRANSFER_EUSART2(0);}					//������ � ������ ���������
														}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;	//����� BODY �� ���������
		case TR_CAL:	//��������� ����������
						if	(LENGHT_INT == 0)	{
							if (ADDR_ADDITIONAL == ADDR_BSI_ACS)	{//�������� ������ �� ����������� ��������� 19.01.2015
							ADRESS_1 = 0x5300;
							ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_ACS_LENGHT, &BUF[8]);
							LENGHT_INT = (unsigned int) CAL_ACS_LENGHT;	TRANSFER_EUSART2(LENGHT_INT);
														}
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;		//����� BODY �� ���������
		case ALLOW_CAL:	//��������� ������
						if	(LENGHT_INT == 0)	{
											TRANSFER_EUSART2(0); FLAG.CAL = ON;
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;		//����� BODY �� ���������	
		case WR_CAL:	//�������� ����������, 126 ����
						if	(LENGHT_INT == CAL_ACS_LENGHT)	{
							if (FLAG.CAL == ON)	{
								LENGHT_INT = (unsigned int) CAL_ACS_LENGHT; TRANSFER_EUSART2(LENGHT_INT);					//����� ���, � body ����� �����������
								ADRESS_1 = 0x5300;	ADRESS_2 = ADRESS_1 + (UINT32) CAL_ACS_LENGHT;
								EraseFlash((UINT32) ADRESS_1 ,(UINT32) ADRESS_2);
								WriteBytesFlash((UINT32) ADRESS_1,(UINT16) CAL_ACS_LENGHT, &BUF[8]); FLAG.CAL = OFF; 
								//ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_LENGHT, &BUF[8]);
								//LEN = (unsigned char) CAL_LENGHT;	reply(LEN);											//�����-���������� ��� ������ ����� ������, �� ��� ��� �����
												}	else {ERROR = err_code[13]; TRANSFER_EUSART2(0);}					//������ � ������ ���������
														}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;	//����� BODY �� ���������

		default:	ERROR = err_code[11]; TRANSFER_EUSART2(0); break;	//�������� ��� ������� ����������������
				}	


}		//����� ACS_WORK(void)
//....................................................................................
void Delay_Us(unsigned char NUM)		//�������� ������� ������������, �� �� ������ 8 mcs
{
	N = NUM - 7;
		_asm
			nop
Delay_Cycle:
			decfsz N, 1, 1
			goto Delay_Cycle
		_endasm
}
//....................................................................................
void Get_An_Tst(void)
	{//Decode=Tool_An_Tst;
//	Sstart=0;Eend=19;Get_An();Tr_Array();
	}
//....................................................................................
void Get_Tnz_Tst(void)
	{//Decode=Tool_An_Tst;
//	Sstart=0;Eend=19;Get_An();Tr_Array();
	}
//....................................................................................
