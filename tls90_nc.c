/*
05.09.2016 - программа для совмещенного модуля ТМ+ТД+Р автономный прибор
этот процессор ведущий
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

#define	Version	0x0E					//номер версии

#define	Dev		1						//делитель для USART 750-0 375-1
#define	Ttime	3						//задержка на 1 такт для USART в мкс тот же
//-----------------

#pragma udata BUFER
unsigned char	BIG_BUF[1152];
#pragma udata

#pragma udata BUFER_BSI
unsigned char	BSI_BUFER[BSI_BUF_SIZE];
#pragma udata

unsigned char	N;				//для delay_us
unsigned char	i;
unsigned char	GLOBAL_ERROR;	//ошибка при сборе данных со всех бси, которая записывается в BUF[3]

unsigned int	START_FLASH_ADDR;

unsigned int	ADRESS_1, ADRESS_2;

unsigned int	Sstart;			//первая ячейка массива
unsigned int	Eend;			//последняя ячейка массива
unsigned int	cnt;
unsigned int	Bit_Test;
unsigned int	DATA;

unsigned char	SLAVE_CAL_LENGHT;

unsigned char   *BUF;			//с буфером работаем через указатель
unsigned char   *BSI_BUF;		//с буфером работаем через указател

union
{
	unsigned int	TYPE_INT;		// слово данных
	unsigned char	TYPE_CHAR[2];	// байты данных
} ARRAY_DATA;

struct
{
unsigned char ID		: 1;
unsigned char CAL		: 1;
unsigned char FLASH_COM	: 1;
}	FLAG;

void TNZ_WORK(void);
void ACS_WORK(void);

void Delay_OneCycle(unsigned char NUM);	//задержка кратностью 1 цикл
//----------------------------------------------------------------------------
void INIT(void)		//начальная инициализация
{

ANCON0 = 0x00;			//все каналы - дискретные
ANCON1 = 0x00;

	TRISA = 0b00011101;	//0 - вход, 1,5 - выход, остальные не используются
	PORTA = OFF;	 
	TXSL = 1;

//---------инициализация PORTC И RS-485 EUSART1 РАБОТА С ПОДЧИНЕННЫМИ БСИ
//	LATCbits.LATC6 = ON; 	//неведомая хрень, чтоб уровень был 1 на usart
//	LATCbits.LATC1 = OFF; 	//неведомая хрень, чтоб уровень был 1 на usart

	TRISC = 0b10010011; PORTC = 0x60;	//0,1 - вход, 4 не исп., 2,3,5,6  - выход, 6,7- uart, RX (RC7) TX (RC6), ReDe (RC5)

	TXSTA1 = 0x06;		//0000 0110
	RCSTA1 = 0x80;		//1000 0000

//	SPBRG1 = (FOSC + BAUDRATE*8) / (BAUDRATE*16) - 1;
	SPBRG1 = Dev;		//делитель для скорости 375 кБод

	RCSTA1bits.SPEN = ON;		//Включить модуль USART 
	RCSTA1bits.CREN = OFF;		//прием 2-х байт возможен
	TXSTA1bits.TXEN = OFF;		//запрет отправки
	BAUDCON1bits.BRG16 = OFF;
//---------инициализация------------- 

//---------инициализация PORTB И  RS-485 EUSART2 РАБОТА С АДАПТЕРОМ
//	LATBbits.LATB6 = ON; 	//неведомая хрень, чтоб уровень был 1 на usart
//	LATBbits.LATB1 = OFF; 	//неведомая хрень, чтоб уровень был 1 на usart
	TRISB = 0b10010101; PORTB = 0x60; 	//0 - вход, 1, 5 - выход 4 не исп., 2,3,5,6  - выход, 6,7- uart RX (RB7) TX (RB6), ReDe (RB5)

	TXAC = 1;

	TXSTA2 = 0x06;		//0000 0110
	RCSTA2 = 0x80;		//1000 0000

//	SPBRG2 = (FOSC + BAUDRATE*8) / (BAUDRATE*16) - 1;
	SPBRG2 = Dev;		//делитель для скорости 375 кБод

	RCSTA2bits.SPEN = ON;		//Включить модуль USART 
	RCSTA2bits.CREN = OFF;		//прием 2-х байт возможен
	TXSTA2bits.TXEN = OFF;		//запрет отправки
	BAUDCON2bits.BRG16 = OFF;
//---------инициализация--------------

	ReDe1 = ON; ReDe2 = OFF;		//На прием команды
	ERROR = 0;
	Bit_Test = 0; LENGHT_INT = 0;
	j=2;

BUF = BIG_BUF;	//определили указатель
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
SIGNAL:	RECIEVE_EUSART2();					//прием команды	
		if ((ADDR_MAIN == BASE_ADDR_LEFT) && (ADDR_ADDITIONAL == BASE_ADDR_RIGHT)) {ADDR_MAIN = ADDR; ADDR_ADDITIONAL = 0x00;}	//Адрес общий для всех
		if (ADDR_MAIN!= ADDR)		 {goto SIGNAL;}			//молчание если несовпадение адреса

		if (ERROR!= 0)	{if (CMD == ECHO) {TRANSFER_EUSART2(LENGHT_INT);}	else	{TRANSFER_EUSART2(0);} goto SIGNAL;}		//Ответ на важные ошибки

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
void GET_ALL()					//Опрос всех БСИ, команда 0x00
{
/*
Последовательность данных:
1. Акселерометры, температура - 38 байт 
2. Натяжение - 6 байт, с двумя каналами 10 байт
3. Резистивиметр - 14 байт
4. Термометр-манометр - 8 байт.
*/
//не забыть здесь вставить обработку ошибку если боди не 2!!!
for (j=0; j<ALL_DATA_SIZE; j++)	{BUF[j+8] = BSI_BUF[j];}
	if (GLOBAL_ERROR!= 0) {ERROR = INCOMPLETE_DATA_ERROR; GLOBAL_ERROR = 0;}	//Ошибка от предыдущих измерений
//ERROR = 0;	//для отладки чтоб не было C0
		LENGHT_INT = ALL_DATA_SIZE; TRANSFER_EUSART2(LENGHT_INT);			//Отправка предыдущего пакета
		for (j=0;j<ALL_DATA_SIZE;j++) {BSI_BUF[j] = 0;} ERROR = 0;			//Занулить ошибку обязательно

//Акселерометры 38 байт
	MARKER = OFF;	Sstart = 2;			Eend = ACS_DATA_SIZE + 1;	GET_ACS();	MARKER = ON;				//[0],[1] ERROR 2-39 позиции в BSI_BUF 8-47 in BUF
	if (ErrorReadByteUsart) {ERROR = err_code[6];}			//нет связи с подчиненным процессором
	if	(ERROR)	{BSI_BUF[Sstart-1] = ERROR; BSI_BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR; ERROR = 0;}		//Занулить ошибку обязательно

//Натяжение - сжатие 10 байт
	MARKER = OFF;	Sstart = Eend + 3;	Eend = Sstart + TNZ_DATA_SIZE - 1;	GET_TNZ();	MARKER = ON;		//CAL_TNZ_LENGHT	162 байт	 48-59 in BUF
	if (ErrorReadByteUsart) {ERROR = err_code[6];}			//нет связи с подчиненным процессором
	if	(ERROR)	{BSI_BUF[Sstart-1] = ERROR; BSI_BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR; ERROR = 0;}		//Занулить ошибку обязательно

//Резистивиметр 14 байт
	ARRAY_DATA.TYPE_INT = ADDR_RB;  CMD = TRANS;	RETRANSFER_EUSART1(2);	RECIEVE_EUSART1();
	Sstart = Eend + 3;	Eend = Sstart + RB_DATA_SIZE - 1;		//7 слов в общем опросе						//[52],[53] ERROR 54-67 позиции в BSI_BUF 60-75 in BUF
//	for	(j=Sstart;j<=Eend;j+=2) {BSI_BUF[j] = BUF[j-45]; BSI_BUF[j+1] = BUF[j-46];}
	for (j=Sstart;j<=Eend;j++) {BSI_BUF[j] = BUF[j-46];}
	if	(ERROR)	{BSI_BUF[Sstart-1] = ERROR; BSI_BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR;}
//	Delay1KTCYx(6);//пауза после резистивиметра 2,6 мс ато не отвечает

//Термометр-манометр 8 байт
	ARRAY_DATA.TYPE_INT = ADDR_TD;	 CMD = TRANS;	RETRANSFER_EUSART1(2);	RECIEVE_EUSART1();
	Sstart = Eend + 3;	Eend =  Sstart + TD_DATA_SIZE - 1;		//4 слова в общем опросе					//[68],[69] ERROR 70-77 позиции в BSI_BUF 76-85 in BUF
//	for	(j=Sstart;j<=Eend;j+=2) {BSI_BUF[j] = BUF[j-59]; BSI_BUF[j+1] = BUF[j-60];}
	for (j=Sstart;j<=Eend;j++) {BSI_BUF[j] = BUF[j-62];}
	if	(ERROR)	{BSI_BUF[Sstart-1] = ERROR; BSI_BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR;}
}
//....................................................................................
void TNZ_WORK(void)
{
switch (CMD)	{
//		case INF:	INF_();		break;
		case ECHO:	TRANSFER_EUSART2(LENGHT_INT);	break;					//эхо-тест, обвязка 12 байт LENGHT_INT = N_BYTES_RECEIVED - 12;
		case TRANS:		for (j=0; j<ALL_DATA_SIZE; j++)	{BUF[j+8] = BSI_BUF[j];}
							if (ERROR == 0)	{
								LENGHT_INT = TNZ_DATA_SIZE; TRANSFER_EUSART2(LENGHT_INT);						//Отправка предыдущего пакета
								for (cnt=Sstart;cnt<=Eend;cnt++) {BSI_BUF[cnt] = 0;}
											} else {TRANSFER_EUSART2(0);}
						MARKER = OFF;	Sstart = 0;	Eend = TNZ_DATA_SIZE - 1;	GET_TNZ();	MARKER = ON;
						if (ErrorReadByteUsart) {ERROR = err_code[6];}									break;	//нет связи с подчиненным процессором	

		case TR_ID:		//прочитать идентификатор
						if	(LENGHT_INT == 0)	{
							if (ADDR_ADDITIONAL == ADDR_BSI_TNZ){					//Проверяем подадрес
							ADRESS_1 = 0x5400;
							ReadFlash((UINT32) ADRESS_1, (UINT16) ID_LENGHT, &BUF[8]);
							LENGHT_INT = (unsigned int) ID_LENGHT;	TRANSFER_EUSART2(LENGHT_INT);
																}
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;

		case ALLOW_ID:												//разрешить запись идентификатора
						for (i=8; i<16;i++)	{						//проверка пароля
						FLAG.ID = ON;
						if (BUF[i] != Password[i-8]) {FLAG.ID = OFF; ERROR = err_code[12];	break;}
											}	TRANSFER_EUSART2(0);	break; 
		case WR_ID:		//записать идентификатор, 128 байт
						if	(LENGHT_INT == ID_LENGHT)	{
							if (FLAG.ID == ON)	{
								LENGHT_INT = (unsigned int) ID_LENGHT; TRANSFER_EUSART2(LENGHT_INT);					//вообщем это пантюхин одобрил, ответ эхо, в body слова перевернуты
								ADRESS_1 = 0x5400;	ADRESS_2 = ADRESS_1 + (UINT32) ID_LENGHT;
								EraseFlash((UINT32) ADRESS_1,(UINT32)ADRESS_2);
								WriteBytesFlash((UINT32)ADRESS_1,(UINT16) ID_LENGHT, &BUF[8]); FLAG.ID = OFF;
								//ReadFlash((UINT32) ADRESS_1, (UINT16) ID_LENGHT, &BUF[8]);							//ответ-идентификатор был раньше после записи, но это тип долго
								//LEN = (unsigned char) ID_LENGHT;	reply(LEN);	
												}	else {ERROR = err_code[13]; TRANSFER_EUSART2(0);}					//Запись в память запрещена
														}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;	//длина BODY не корректна
		case TR_CAL:	//прочитать калибровку
						if	(LENGHT_INT == 0)	{
							if (ADDR_ADDITIONAL == ADDR_BSI_TNZ)	{//отвечает только за центральный процессор 19.01.2015
							ADRESS_1 = 0x5500;
							ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_TNZ_LENGHT, &BUF[8]);
							LENGHT_INT = (unsigned int) CAL_TNZ_LENGHT;	TRANSFER_EUSART2(LENGHT_INT);
																	}
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;		//длина BODY не корректна
		case ALLOW_CAL:	//разрешить запись
						if	(LENGHT_INT == 0)	{
											TRANSFER_EUSART2(0); FLAG.CAL = ON;
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;		//длина BODY не корректна	
		case WR_CAL:	//записать калибровку, 126 байт
					if	(LENGHT_INT == CAL_TNZ_LENGHT)	{
						if (FLAG.CAL == ON)	{
							LENGHT_INT = (unsigned int) CAL_TNZ_LENGHT; TRANSFER_EUSART2(LENGHT_INT);					//ответ эхо, в body слова перевернуты
							ADRESS_1 = 0x5500;	ADRESS_2 = ADRESS_1 + (UINT32) CAL_TNZ_LENGHT;
							EraseFlash((UINT32) ADRESS_1 ,(UINT32) ADRESS_2);
							WriteBytesFlash((UINT32) ADRESS_1,(UINT16) CAL_TNZ_LENGHT, &BUF[8]); FLAG.CAL = OFF; 
							//ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_LENGHT, &BUF[8]);
							//LEN = (unsigned char) CAL_LENGHT;	reply(LEN);											//ответ-калибровка был раньше после записи, но это тип долго
											}	else {ERROR = err_code[13]; TRANSFER_EUSART2(0);}					//Запись в память запрещена
														}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;	//длина BODY не корректна

		default:	ERROR = err_code[11]; TRANSFER_EUSART2(0); break;	//Заданный код команды неподдерживается
				}	

}		//Конец TNZ_WORK(void)
//============================================================================================================================
void ACS_WORK(void)
{
switch (CMD)	{
//		case INF:	INF_();		break;
		case ECHO:	TRANSFER_EUSART2(LENGHT_INT);	break;					//эхо-тест, обвязка 12 байт LENGHT_INT = N_BYTES_RECEIVED - 12;
		case TRANS:		for (j=0; j<ALL_DATA_SIZE; j++)	{BUF[j+8] = BSI_BUF[j];}	//передать данные
							if (ERROR == 0)	{
							LENGHT_INT = ACS_DATA_SIZE; TRANSFER_EUSART2(LENGHT_INT);	
							for (cnt=Sstart;cnt<=Eend;cnt++) {BSI_BUF[cnt] = 0;}
											} else {TRANSFER_EUSART2(0);}						//Отправка предыдущего пакета
							MARKER = OFF;	Sstart = 0;		Eend = ACS_DATA_SIZE - 1 ;	GET_ACS();	MARKER = ON;
							if (ErrorReadByteUsart) {ERROR = err_code[6];}			break;		//нет связи с подчиненным процессором

		case TR_ID:		//прочитать идентификатор
						if	(LENGHT_INT == 0)	{
							if (ADDR_ADDITIONAL == ADDR_BSI_ACS){					//Проверяем подадрес
							ADRESS_1 = 0x5200;
							ReadFlash((UINT32) ADRESS_1, (UINT16) ID_LENGHT, &BUF[8]);
							LENGHT_INT = (unsigned int) ID_LENGHT;	TRANSFER_EUSART2(LENGHT_INT);
																}
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;

		case ALLOW_ID:												//разрешить запись идентификатора
						for (i=8; i<16;i++)	{						//проверка пароля
						FLAG.ID = ON;
						if (BUF[i] != Password[i-8]) {FLAG.ID = OFF; ERROR = err_code[12];	break;}
											}	TRANSFER_EUSART2(0);	break; 
		case WR_ID:		//записать идентификатор, 128 байт
						if	(LENGHT_INT == ID_LENGHT)	{
							if (FLAG.ID == ON)	{
								LENGHT_INT = (unsigned int) ID_LENGHT; TRANSFER_EUSART2(LENGHT_INT);					//вообщем это пантюхин одобрил, ответ эхо, в body слова перевернуты
								ADRESS_1 = 0x5200;	ADRESS_2 = ADRESS_1 + (UINT32) ID_LENGHT;
								EraseFlash((UINT32) ADRESS_1,(UINT32)ADRESS_2);
								WriteBytesFlash((UINT32)ADRESS_1,(UINT16) ID_LENGHT, &BUF[8]); FLAG.ID = OFF;
								//ReadFlash((UINT32) ADRESS_1, (UINT16) ID_LENGHT, &BUF[8]);							//ответ-идентификатор был раньше после записи, но это тип долго
								//LEN = (unsigned char) ID_LENGHT;	reply(LEN);	
												}	else {ERROR = err_code[13]; TRANSFER_EUSART2(0);}					//Запись в память запрещена
														}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;	//длина BODY не корректна
		case TR_CAL:	//прочитать калибровку
						if	(LENGHT_INT == 0)	{
							if (ADDR_ADDITIONAL == ADDR_BSI_ACS)	{//отвечает только за центральный процессор 19.01.2015
							ADRESS_1 = 0x5300;
							ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_ACS_LENGHT, &BUF[8]);
							LENGHT_INT = (unsigned int) CAL_ACS_LENGHT;	TRANSFER_EUSART2(LENGHT_INT);
														}
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;		//длина BODY не корректна
		case ALLOW_CAL:	//разрешить запись
						if	(LENGHT_INT == 0)	{
											TRANSFER_EUSART2(0); FLAG.CAL = ON;
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;		//длина BODY не корректна	
		case WR_CAL:	//записать калибровку, 126 байт
						if	(LENGHT_INT == CAL_ACS_LENGHT)	{
							if (FLAG.CAL == ON)	{
								LENGHT_INT = (unsigned int) CAL_ACS_LENGHT; TRANSFER_EUSART2(LENGHT_INT);					//ответ эхо, в body слова перевернуты
								ADRESS_1 = 0x5300;	ADRESS_2 = ADRESS_1 + (UINT32) CAL_ACS_LENGHT;
								EraseFlash((UINT32) ADRESS_1 ,(UINT32) ADRESS_2);
								WriteBytesFlash((UINT32) ADRESS_1,(UINT16) CAL_ACS_LENGHT, &BUF[8]); FLAG.CAL = OFF; 
								//ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_LENGHT, &BUF[8]);
								//LEN = (unsigned char) CAL_LENGHT;	reply(LEN);											//ответ-калибровка был раньше после записи, но это тип долго
												}	else {ERROR = err_code[13]; TRANSFER_EUSART2(0);}					//Запись в память запрещена
														}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;	//длина BODY не корректна

		default:	ERROR = err_code[11]; TRANSFER_EUSART2(0); break;	//Заданный код команды неподдерживается
				}	


}		//Конец ACS_WORK(void)
//....................................................................................
void Delay_Us(unsigned char NUM)		//задержка кратная микросекунде, но не меннее 8 mcs
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
