#include <RS485.h>
#include <CRC.h>
#include <flash.h>

const rom unsigned char err_code[]	=	{0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x31, 0x41, 0x43, 0x44, 0x45, 0x46, 0x48, 0x49, 0x4B, 0x4D, 0x52, 0x61};
const rom unsigned char MNEM[]	=		{'M', 'T', 'K', 'H'};								//мнемоника
const rom unsigned char MADD[]	=		{'N', 'O', 'N', 'E'};								//NONE
const rom unsigned char Password[]	=	{0x50, 0x61, 0x73, 0x73, 0x77, 0x6F, 0x72, 0x64};	//Password

unsigned char	ADDR_MAIN;				//Адрес BUF[0]
unsigned char	ADDR_ADDITIONAL;		//Адрес BUF[1] Дополнительный может быть 0x00 (ALL), 0x01 (TNZ), 0x02 (ACS), 0x03 (REZ), 0x04 (TD)

unsigned char	ERROR;					//Байт ошибок
unsigned char	CLEAR_RCREG;			//Для зачистки буферов UART
unsigned int	CMD;					//Команда BUF[2], BUF[3] 16 бит
unsigned int	j;						//Счетчик 16 бит
unsigned int	LENGHT_INT;				//Длина 16 бит BUF[6], BUF[7]
unsigned int	N_BYTES_RECEIVED;		//Количество принятых байт 16 бит, LENGHT_INT + 12
unsigned int	TEMP_INT;
//..................................................EUSART2........................
//.................................................................................
void RECIEVE_EUSART2(void)							//Работа с адаптером
{
	MARKER = OFF;			//во внутренних процедурах скидываем, а по завершении поднимаем

//Разница в том, что он работает на прием, поэтому нет таймера на ожидание ответа
	ReDe2 = OFF;	ERROR = 0;				//должно быть перед включением приема!
	TXSTA2bits.TXEN = OFF;					//запрет отправки

	RCSTA2bits.CREN = ON;	//ожидание приема
//timer_
	T0CON = 0b00000000;	INTCONbits.TMR0IF = OFF;	//set up timer0 - prescaler 1:2, 16-bit timer, timer OFF (7 bit T0CON)
//timer__

	//прием команды
	for (j=0; j<=BUF_SIZE; j++)
	{
	if (j == BUF_SIZE) {j = 1; ERROR = err_code[8]; goto BREAK_LOOP_RECIEVE_EUSART2;}					//буфер переполнен
	MARKER = ON;
	while(PIR3bits.RC2IF == OFF) {if (INTCONbits.TMR0IF) goto BREAK_LOOP_RECIEVE_EUSART2;}; BUF[j] = RCREG2;
	MARKER = OFF;
	if (RCSTA2bits.OERR) {CLEAR_RCREG = RCREG2; CLEAR_RCREG = RCREG2; CLEAR_RCREG = RCREG2; RCSTA2bits.CREN = OFF; RCSTA2bits.CREN = ON;}
	if (RCSTA2bits.FERR) {CLEAR_RCREG = RCREG2;}
	TMR0L = TurnTimer0-50;	TMR0H = 0xFF; T0CONbits.TMR0ON = ON;	//устанавливаем таймер на 73 мкс до срабатывания 
	}

BREAK_LOOP_RECIEVE_EUSART2: INTCONbits.TMR0IF = OFF; N_BYTES_RECEIVED = j; T0CONbits.TMR0ON = OFF;

	CMD = 0; LENGHT_INT = 0; 		//Обнуляем перед разбором
	RCSTA2bits.CREN = OFF;

	ADDR_MAIN = BUF[0];	ADDR_ADDITIONAL = BUF[1];					//Расставляем адреса по переменным 
	CMD = BUF[2]; CMD<<= 8; CMD+= BUF[3];							//Получаем команду в нужной последовательности
	LENGHT_INT = BUF[6]; LENGHT_INT<<=8; LENGHT_INT+= BUF[7];		//Вычиляем длину BUF[6], BUF[7] 

	if (ERROR == err_code[8])	{BUF[1] = 0; N_BYTES_RECEIVED = 0; ADDR_ADDITIONAL = 0;}		 		//первый приоритет
		else 	{
			Bit_Test =  BUF[N_BYTES_RECEIVED-4]; Bit_Test<<=8; Bit_Test+=BUF[N_BYTES_RECEIVED-3];		// для проверки crc
			TEMP_INT = N_BYTES_RECEIVED - 4; CRC_16 = getCrc16(&BUF[8], LENGHT_INT);

	if (Bit_Test != CRC_16) {ERROR = err_code[10];}					//ошибка CRC
	if ((BUF[N_BYTES_RECEIVED-1] != EOL_RIGHT) || (BUF[N_BYTES_RECEIVED-2] != EOL_LEFT)) {ERROR = err_code[7];}	//ошибка отсутствие EOL
	if (LENGHT_INT!=(N_BYTES_RECEIVED-12)) {ERROR = err_code[15];}	//длина не соответсвует LEN
	if (LENGHT_INT&1) {ERROR = err_code[9];}						//нечетное LEN
	BUF[3] = ERROR;													//В ответе ERROR на месте BUF[3]
				}

	Bit_Test = 0; //LENGHT_INT = 0;

	MARKER = ON;
}
//.................................................................................
void TRANSFER_EUSART2(unsigned int LENGHT)			//Работа с адаптером
{
	unsigned int TEMP;
	MARKER = OFF;															//вот внутренних процедурах скидываем, а по завершении поднимаем
			BUF[0] = ADDR | 0x7; BUF[1] = ADDR_ADDITIONAL;					//Адрес
			BUF[2] = 0; BUF[3] = ERROR;										//Ошибки
			TEMP = LENGHT + 8;												//добавляем ADDR и LENGHT
			BUF[4] = 0; BUF[5] = 0;											//Длина
			BUF[7] = LENGHT; LENGHT>>=8; BUF[6] = LENGHT;																				
			CRC_16 = getCrc16(&BUF[8], TEMP-8);								//CRC16 считается только по Body 16.09.2016	
			BUF[TEMP+1] = CRC_16; CRC_16>>=8; BUF[TEMP] = CRC_16;
			BUF[TEMP+2] = EOL_LEFT;	BUF[TEMP+3] = EOL_RIGHT;

			TXSTA2bits.TXEN = ON; ReDe2 = ON; TEMP+= 4;
			PIR3bits.TX2IF = ON;
				for (j=0; j<TEMP; j++)	{TXREG2 = BUF[j];	NOP();	while(PIR3bits.TX2IF == 0);}

			TXSTA2bits.TXEN = OFF;	ReDe2 = OFF;
	MARKER = ON;
}
//....................................................................................
void TRANSFER_EUSART2_INV(unsigned int LENGHT)		//передача данных rs-485 c перевернутыми словами в body
{
	unsigned int TEMP;
	unsigned char Rev1, Rev2;
	MARKER = OFF;															//вот внутренних процедурах скидываем, а по завершении поднимаем
			BUF[0] = ADDR | 0x7; BUF[1] = ADDR_ADDITIONAL;					//Адрес
			BUF[2] = 0; BUF[3] = ERROR;										//Ошибки
			TEMP = LENGHT + 8;												//добавляем ADDR и LENGHT
			BUF[4] = 0; BUF[5] = 0;											//Длина
			BUF[7] = LENGHT; LENGHT>>=8; BUF[6] = LENGHT;
			for (j=8; j<TEMP; j+=2) {Rev1 = BUF[j]; Rev2 = BUF[j+1]; BUF[j] = Rev2; BUF[j+1] = Rev1;}	//крутим body
			CRC_16 = getCrc16(&BUF[8], TEMP-8);								//CRC16 считается только по Body 16.09.2016	
			BUF[TEMP+1] = CRC_16; CRC_16>>=8; BUF[TEMP] = CRC_16;
			BUF[TEMP+2] = EOL_LEFT;	BUF[TEMP+3] = EOL_RIGHT;

			TXSTA2bits.TXEN = ON; ReDe2 = ON; TEMP+= 4;
			PIR3bits.TX2IF = ON;
				for (j=0; j<TEMP; j++)	{TXREG2 = BUF[j];	NOP();	while(PIR3bits.TX2IF == 0);}

			TXSTA2bits.TXEN = OFF;	ReDe2 = OFF;
	MARKER = ON;

	TEMP-=4; for (j=8; j<TEMP; j+=2) {Rev2 = BUF[j+1]; Rev1 = BUF[j]; BUF[j] = Rev2; BUF[j+1] = Rev1;}	//Возвращаем на места
}
//..................................................EUSART1........................
//.................................................................................
void RECIEVE_EUSART1(void)							//Работа с БСИ
{
	MARKER = OFF;	//во внутренних процедурах скидываем, а по завершении поднимаем

//	CMD = 0;
	LENGHT_INT = 0; ERROR = 0;				//Обнуляем ключевые параметры
	ReDe1 = OFF;									//должно быть перед включением приема!
	TXSTA1bits.TXEN = OFF; 							//запрет отправки
//timer_
	TMR0H = 0xE2; TMR0L = 0xB3;						//5ms до срабатывания на команду
	if ((CMD == WR_CAL) || (CMD == WR_ID)) {TMR0H = 0x00; TMR0L = 0x01;}//45ms на запись калибровки (идентификатора), так как ответ калибровка(идетнификатор), считанная с памяти
	CMD = 0;										//обнуляем после того как определили время ожидания ответа
//	TMR0H = 0xA8; TMR0L = 0x1B;						//15ms до срабатывания на команду, для записи идентификатора
//	TMR0H = 0xE2; TMR0L = 0xB3;						//5ms до срабатывания на команду
	T0CON = 0b10000000;	INTCONbits.TMR0IF = OFF;	//set up timer0 - prescaler 1:2, 16-bit timer, timer ON (7 bit T0CON)
//timer__
	RCSTA1bits.CREN = ON;	//ожидание приема
	//прием ответа
	while(PIR1bits.RC1IF == OFF) {if (INTCONbits.TMR0IF)	{ERROR = err_code[6];	goto BREAK_LOOP_RECIEVE_EUSART1;}}; BUF[0] = RCREG1;	//защита от неответа
	TMR0L = TurnTimer0-130;	TMR0H = 0xFF;	//приняли первый байт, переключаем таймер на 126 mcs

	for (j=1; j<=BUF_SIZE; j++)
	{
	if (j == BUF_SIZE) {j = 1; ERROR = err_code[8]; goto BREAK_LOOP_RECIEVE_EUSART1;}

	while(PIR1bits.RC1IF == OFF) {if (INTCONbits.TMR0IF) goto BREAK_LOOP_RECIEVE_EUSART1;}; BUF[j] = RCREG1;
	if (RCSTA1bits.OERR) {CLEAR_RCREG = RCREG1; CLEAR_RCREG = RCREG1; CLEAR_RCREG = RCREG1; RCSTA1bits.CREN = OFF; RCSTA1bits.CREN = ON;}

	TMR0L = TurnTimer0-30;	TMR0H = 0xFF;	//приняли первый байт, переключаем таймер на 60 mcs
	}

BREAK_LOOP_RECIEVE_EUSART1: INTCONbits.TMR0IF = OFF; N_BYTES_RECEIVED = j; T0CONbits.TMR0ON = OFF;

	RCSTA1bits.CREN = OFF;

	ADDR_MAIN = BUF[0];	ADDR_ADDITIONAL = BUF[1];					//Расставляем адреса по переменным 
	CMD = BUF[2]; CMD<<= 8; CMD+= BUF[3];							//Получаем команду в нужной последовательности
	LENGHT_INT = BUF[6]; LENGHT_INT<<=8; LENGHT_INT+= BUF[7];		//Вычиляем длину BUF[6], BUF[7] 

	if (ERROR!= err_code[6]) {						// 0x31 - Нет связи с подчиненным контроллером

	if (ERROR == err_code[8])	{BUF[1] = 0; N_BYTES_RECEIVED = 0; ADDR_ADDITIONAL = 0;}		 //первый приоритет
		else {
			Bit_Test = BUF[N_BYTES_RECEIVED-4]; Bit_Test<<= 8; Bit_Test+= BUF[N_BYTES_RECEIVED-3];		// для проверки crc
			TEMP_INT = N_BYTES_RECEIVED - 4; CRC_16 = getCrc16(&BUF[8], LENGHT_INT);

			if (Bit_Test != CRC_16) {ERROR = err_code[10];}					//ошибка CRC
			if ((BUF[N_BYTES_RECEIVED-1] != EOL_RIGHT) || (BUF[N_BYTES_RECEIVED-2] != EOL_LEFT)) {ERROR = err_code[7];}	//ошибка отсутствие EOL
			if (LENGHT_INT!= (N_BYTES_RECEIVED-12)) {ERROR = err_code[15];}	//длина не соответсвует LEN
			if (LENGHT_INT&1) {ERROR = err_code[9];}						//нечетное LEN
			BUF[3] = ERROR; 												//В ответе ERROR на месте BUF[3]
			}
							}						// завершение первой части обработки ошибок
	Bit_Test = 0; //LENGHT_INT = 0;
	MARKER = ON;
}
//.................................................................................
void TRANSFER_EUSART1(unsigned int LENGHT)			//Работа с БСИ
{
	unsigned int TEMP;
	MARKER = OFF;																//вот внутренних процедурах скидываем, а по завершении поднимаем
			BUF[0] = ARRAY_DATA.TYPE_CHAR[1]; BUF[1] = ARRAY_DATA.TYPE_CHAR[0];	//Здесь уже должен быть подставлен адрес БСИ
			BUF[2] = 0; BUF[3] = 0;												//BUF[3] обычно ошибки, но в данном случаем я шлю команду БСИ
			TEMP = LENGHT + 8;													//добавляем ADDR и LENGHT
			BUF[4] = 0; BUF[5] = 0;												//Длина
			BUF[7] = LENGHT; LENGHT>>=8; BUF[6] = LENGHT;																				
			CRC_16 = getCrc16(&BUF[8], TEMP-8);									//CRC16 считается только по Body 16.09.2016	
			BUF[TEMP+1] = CRC_16; CRC_16>>=8; BUF[TEMP] = CRC_16;
			BUF[TEMP+2] = EOL_LEFT;	BUF[TEMP+3] = EOL_RIGHT;

			TXSTA1bits.TXEN = ON; ReDe1 = ON; TEMP+=4;
			PIR1bits.TX1IF = ON;

				for (j=0; j<TEMP; j++)	{TXREG1 = BUF[j];	NOP();	while(PIR1bits.TX1IF == 0);}//пока не поднимится флаг, означающий что регистр пуст

			TXSTA1bits.TXEN = OFF;	ReDe1 = OFF;
	MARKER = ON;
}
//.................................................................................
void RETRANSFER_EUSART1(unsigned int LENGHT)			//Работа с БСИ
{
	unsigned int TEMP;
if (CMD == TRANS) {BUF[8] = 0; BUF[9] = 0;}
	MARKER = OFF;																//вот внутренних процедурах скидываем, а по завершении поднимаем
			BUF[0] = ARRAY_DATA.TYPE_CHAR[1]; BUF[1] = ARRAY_DATA.TYPE_CHAR[0];	//Здесь уже должен быть подставлен адрес БСИ
			BUF[2] = CMD>>8; BUF[3] = CMD;										//BUF[3] обычно ошибки, но в данном случаем я шлю команду БСИ
			TEMP = LENGHT + 8;													//добавляем ADDR и LENGHT
			BUF[4] = 0; BUF[5] = 0;												//Длина
			BUF[7] = LENGHT; LENGHT>>=8; BUF[6] = LENGHT;																				
			CRC_16 = getCrc16(&BUF[8], TEMP-8);									//CRC16 считается только по Body 16.09.2016	
			BUF[TEMP+1] = CRC_16; CRC_16>>=8; BUF[TEMP] = CRC_16;
			BUF[TEMP+2] = EOL_LEFT;	BUF[TEMP+3] = EOL_RIGHT;

			TXSTA1bits.TXEN = ON; ReDe1 = ON; TEMP+=4;
			PIR1bits.TX1IF = ON;

				for (j=0; j<TEMP; j++)	{TXREG1 = BUF[j];	NOP();	while(PIR1bits.TX1IF == 0);}//пока не поднимится флаг, означающий что регистр пуст

			TXSTA1bits.TXEN = OFF; ReDe1 = OFF;
	MARKER = ON;
}
//..........................WORKING................................................
//.................................................................................
void ADAPTER_COMMAND_WORK(void)					//обработка команды адаптера
{

switch (CMD)	{

		case INF:	INF_();		break;
		case TRANS:	GET_ALL();	break;
		case ECHO:	LENGHT_INT = N_BYTES_RECEIVED - 12; TRANSFER_EUSART2(LENGHT_INT);	break;					//эхо-тест, обвязка 12 байт
		case TR_ID:	//прочитать идентификатор
					if	(LENGHT_INT == 0)	{
						if (ADDR_ADDITIONAL == 0x00){				//отвечает только за центральный процессор 19.01.2015
						ADRESS_1 = 0x5000;
						ReadFlash((UINT32) ADRESS_1, (UINT16) ID_LENGHT, &BUF[8]);
						LENGHT_INT = (unsigned int) ID_LENGHT;	TRANSFER_EUSART2(LENGHT_INT);
													}
											}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;

		case ALLOW_ID:												//разрешить запись идентификатора
						for (i=8; i<16;i++)	{						//проверка пароля
						FLAG.ID = ON;
						if (BUF[i] != Password[i-8]) {FLAG.ID = OFF; ERROR = err_code[12];	break;}
											}	TRANSFER_EUSART2(0);	break; 
		case WR_ID:	//записать идентификатор, 128 байт
					if	(LENGHT_INT == ID_LENGHT)	{
						if (FLAG.ID == ON)	{
							LENGHT_INT = (unsigned int) ID_LENGHT; TRANSFER_EUSART2(LENGHT_INT);					//ответ эхо, в body слова перевернуты
							ADRESS_1 = 0x5000;	ADRESS_2 = ADRESS_1 + (UINT32) ID_LENGHT;
							EraseFlash((UINT32) ADRESS_1,(UINT32)ADRESS_2);
							WriteBytesFlash((UINT32)ADRESS_1,(UINT16) ID_LENGHT, &BUF[8]); FLAG.ID = OFF;
							//ReadFlash((UINT32) ADRESS_1, (UINT16) ID_LENGHT, &BUF[8]);							//ответ-идентификатор был раньше после записи, но это тип долго
							//LEN = (unsigned char) ID_LENGHT;	reply(LEN);	
											}	else {ERROR = err_code[13]; TRANSFER_EUSART2(0);}					//Запись в память запрещена
													}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;	//длина BODY не корректна

		case TR_CAL:	//прочитать калибровку
						if	(LENGHT_INT == 0)	{
							if (ADDR_ADDITIONAL==0x00)	{//отвечает центральный процессор всеми калибровками 25.10.2016
							for (j=0;j<33;j++) {BUF[j] = 0;}		//чтоб небыло мусора в месте калибровочных коэффичиентов
							BUF[12] = 0x8E; BUF[13] = 0x02;			//654 длина калибровки минус длина коэфф. - длина длины калибровки)
			//Калибровка акселерометров
							GLOBAL_ERROR = 0;
							ERROR = 0; MARKER = OFF; Sstart = SHIFT_BUF;	Eend = Sstart + CAL_ACS_LENGHT - 1;	//SHIFT_BUF это нужный сдивиг, команда квитанци 8 байт + калибровочные коэффициенты MTKH 8 байт, 2 байта для ошибки
							ADRESS_1 = 0x5300;	ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_ACS_LENGHT, &BUF[Sstart]);				//[16],[17] ERROR, CAL_ACS_LENGHT 122 байт, 18-139 in BUF
							if	(ERROR)	{BUF[Sstart-1] = ERROR; BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR;} else {BUF[Sstart-1] = 0x00; BUF[Sstart-2] = 0x00;}
			//Калибровка натяжения-сжатия
							Sstart = Eend + 3;	Eend = Sstart + CAL_TNZ_LENGHT - 1;
							ADRESS_1 = 0x5500;	ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_TNZ_LENGHT, &BUF[Sstart]);				//[140],[141] ERROR, CAL_TNZ_LENGHT 340 байт, 142-481 in BUF
							if	(ERROR)	{BUF[Sstart-1] = ERROR; BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR;} else {BUF[Sstart-1] = 0x00; BUF[Sstart-2] = 0x00;}
			//Калибровка резистивиметра
							Sstart = Eend + 3;	Eend = Sstart + CAL_RB_LENGHT - 1;
							ARRAY_DATA.TYPE_INT = ADDR_RB;  CMD = TR_CAL;	TRANSFER_CAL_EUSART1(0);	RECIEVE_CAL_EUSART1();			//[482],[483] ERROR, CAL_RB_LENGHT 46 байт, 484-529 in BUF
							if	(ERROR)	{BUF[Sstart-1] = ERROR; BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR;} else {BUF[Sstart-1] = 0x00; BUF[Sstart-2] = 0x00;}
							for (j=Sstart;j<=Eend;j++) {BUF[j] = BSI_BUF[j-476];}
			//Калибровка термометра-манометра
							Sstart = Eend + 3;	Eend =  Sstart + CAL_TD_LENGHT - 1;
							ARRAY_DATA.TYPE_INT = ADDR_TD;	 CMD = TR_CAL;	TRANSFER_CAL_EUSART1(0);	RECIEVE_CAL_EUSART1();			//[530],[531] ERROR, CAL_TD_LENGHT 138 байт, 532-669 in BUF
							if	(ERROR)	{BUF[Sstart-1] = ERROR; BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR;} else {BUF[Sstart-1] = 0x00; BUF[Sstart-2] = 0x00;}
							for (j=Sstart;j<=Eend;j++) {BUF[j] = BSI_BUF[j-524];}

							LENGHT_INT = CAL_ALL_LENGHT; ADDR_MAIN = ADDR; ADDR_ADDITIONAL = 0x00;					//выставляем начальную точку в массиве и длину
							if (GLOBAL_ERROR) {ERROR = INCOMPLETE_DATA_ERROR;} else {ERROR = 0;}								
//ERROR = 0;
							TRANSFER_EUSART2(LENGHT_INT);											//отправка всего
							for (j=0;j<BSI_BUF_SIZE;j++) {BSI_BUF[j] = 0;}
														}
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;		//длина BODY не корректна

/*		Убрали общую калибровку, оставили только чтение
		case TR_CAL:	//прочитать калибровку
						if	(LENGHT_INT == 0)	{
							if (ADDR_ADDITIONAL==0x00)	{//отвечает только за центральный процессор 19.01.2015
							ADRESS_1 = 0x5100;
							ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_LENGHT, &BUF[8]);
							LENGHT_INT = (unsigned int) CAL_LENGHT;	TRANSFER_EUSART2(LENGHT_INT);
														}
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;		//длина BODY не корректна
		case ALLOW_CAL:	//разрешить запись
						if	(LENGHT_INT == 0)	{
											TRANSFER_EUSART2(0); FLAG.CAL = ON;
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;		//длина BODY не корректна	
		case WR_CAL:	//записать калибровку, 194 байт
					if	(LENGHT_INT == CAL_LENGHT)	{
						if (FLAG.CAL == ON)	{
							LENGHT_INT = (unsigned int) CAL_LENGHT; TRANSFER_EUSART2_INV(LENGHT_INT);				//вообщем это пантюхин одобрил, ответ эхо, в body слова перевернуты
							ADRESS_1 = 0x5100;	ADRESS_2 = ADRESS_1 + (UINT32) CAL_LENGHT;
							EraseFlash((UINT32) ADRESS_1 ,(UINT32) ADRESS_2);
							WriteBytesFlash((UINT32) ADRESS_1,(UINT16) CAL_LENGHT, &BUF[8]); FLAG.CAL = OFF; 
							//ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_LENGHT, &BUF[8]);
							//LEN = (unsigned char) CAL_LENGHT;	reply(LEN);											//ответ-калибровка был раньше после записи, но это тип долго
											}	else {ERROR = err_code[13]; TRANSFER_EUSART2(0);}					//Запись в память запрещена
													}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;	//длина BODY не корректна
*///Убрали общую калибровку
		default:	ERROR = err_code[11]; TRANSFER_EUSART2(0); break;	//Заданный код команды неподдерживается
				}	
	for (j=0;j<N_BYTES_RECEIVED;j++) {BUF[j] = 0;}	//очистка буфера
}	//Конец процедуры ADAPTER_COMMAND_WORK
//.................................................................................
void BSI_COMMAND_WORK(void)		//Работа с БСИ по второму USART	
{
//	TRANSFER_EUSART2(LENGHT_INT);
	RETRANSFER_EUSART1(LENGHT_INT);	RECIEVE_EUSART1();
	
	if (ERROR == err_code[6]) {TRANSFER_EUSART2(0); return;}	

		switch (ARRAY_DATA.TYPE_INT)	{

			case ADDR_RB:		ADDR_ADDITIONAL =  ADDR_BSI_RB;		 TRANSFER_EUSART2(LENGHT_INT);	break;
			case ADDR_TD:		ADDR_ADDITIONAL = ADDR_BSI_TD;		 TRANSFER_EUSART2(LENGHT_INT);	break;

			default:	ERROR = err_code[11]; TRANSFER_EUSART2(0); break;	//Заданный код команды неподдерживается
										}

}	//Конец процедуры BSI_COMMAND_WORK

//....................................................................................
void INF_(void)							//команда от адаптера о запросе информации о приборе
{		
if (LENGHT_INT == 0)	{
				for (j=8; j<12; j++)	{BUF[j] = MNEM[j-8];}
				for (j=12; j<16; j++)	{BUF[j] = MADD[j-12];}
				ReadFlash((UINT32)0x5032,(UINT16) 2, &BUF[16]);					//номер хранится со сдвигом на 50 байт от начала таблицы
				BUF[18] = ADDR; BUF[19] = 0x00; 								//Адрес прибора
				BUF[20] = 0x00; BUF[21] = 0x0F; 								//Маска подчиненных бси
				LENGHT_INT = 14;	TRANSFER_EUSART2(LENGHT_INT);
						}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	//длина BODY не корректна
}
//.....................................................................................
void RECIEVE_CAL_EUSART1(void)							//Работа принять калибровку от подчиненного БСИ
{
	MARKER = OFF;	//во внутренних процедурах скидываем, а по завершении поднимаем

	//CMD = 0; 
	LENGHT_INT = 0; ERROR = 0;				//Обнуляем ключевые параметры
	ReDe1 = OFF;									//должно быть перед включением приема!
	TXSTA1bits.TXEN = OFF; 							//запрет отправки
//timer_
	TMR0H = 0xE2; TMR0L = 0xB3;						//5ms до срабатывания на команду
	if ((CMD == WR_CAL) || (CMD == WR_ID)) {TMR0H = 0x00; TMR0L = 0x01;}//45ms на запись калибровки (идентификатора), так как ответ калибровка(идетнификатор), считанная с памяти
	CMD = 0;										//обнуляем после того как определили время ожидания ответа
//	TMR0H = 0xA8; TMR0L = 0x1B;						//15ms до срабатывания на команду, для записи идентификатора
//	TMR0H = 0xE2; TMR0L = 0xB3;						//5ms до срабатывания на команду
	T0CON = 0b10000000;	INTCONbits.TMR0IF = OFF;	//set up timer0 - prescaler 1:2, 16-bit timer, timer ON (7 bit T0CON)
//timer__
	RCSTA1bits.CREN = ON;	//ожидание приема

	//прием ответа
	while(PIR1bits.RC1IF == OFF) {if (INTCONbits.TMR0IF)	{ERROR = err_code[6];	goto BREAK_LOOP_RECIEVE_CAL_EUSART1;}}; BSI_BUF[0] = RCREG1;	//защита от неответа
	TMR0L = TurnTimer0-130;	TMR0H = 0xFF;	//приняли первый байт, переключаем таймер на 126 mcs

	for (j=1; j<=BSI_BUF_SIZE; j++)
	{
	if (j == BSI_BUF_SIZE) {j = 1; ERROR = err_code[8]; goto BREAK_LOOP_RECIEVE_CAL_EUSART1;}

	while(PIR1bits.RC1IF == OFF) {if (INTCONbits.TMR0IF) goto BREAK_LOOP_RECIEVE_CAL_EUSART1;}; BSI_BUF[j] = RCREG1;
	if (RCSTA1bits.OERR) {CLEAR_RCREG = RCREG1; CLEAR_RCREG = RCREG1; CLEAR_RCREG = RCREG1; RCSTA1bits.CREN = OFF; RCSTA1bits.CREN = ON;}

	TMR0L = TurnTimer0-30;	TMR0H = 0xFF;	//приняли первый байт, переключаем таймер на 60 mcs
	}

BREAK_LOOP_RECIEVE_CAL_EUSART1: INTCONbits.TMR0IF = OFF; N_BYTES_RECEIVED = j; T0CONbits.TMR0ON = OFF;

	RCSTA1bits.CREN = OFF;

	ADDR_MAIN = BSI_BUF[0];	ADDR_ADDITIONAL = BSI_BUF[1];					//Расставляем адреса по переменным 
	CMD = BSI_BUF[2]; CMD<<= 8; CMD+= BSI_BUF[3];							//Получаем команду в нужной последовательности
	LENGHT_INT = BSI_BUF[6]; LENGHT_INT<<=8; LENGHT_INT+= BSI_BUF[7];		//Вычиляем длину BUF[6], BUF[7] 

	if (ERROR!= err_code[6]) {						// 0x31 - Нет связи с подчиненным контроллером

	if (ERROR == err_code[8])	{BSI_BUF[1] = 0; N_BYTES_RECEIVED = 0; ADDR_ADDITIONAL = 0;}		 //первый приоритет
		else {
			Bit_Test = BSI_BUF[N_BYTES_RECEIVED-4]; Bit_Test<<= 8; Bit_Test+= BSI_BUF[N_BYTES_RECEIVED-3];		// для проверки crc
			TEMP_INT = N_BYTES_RECEIVED - 4; CRC_16 = getCrc16(&BSI_BUF[8], LENGHT_INT);

			if (Bit_Test != CRC_16) {ERROR = err_code[10];}					//ошибка CRC
			if ((BSI_BUF[N_BYTES_RECEIVED-1] != EOL_RIGHT) || (BSI_BUF[N_BYTES_RECEIVED-2] != EOL_LEFT)) {ERROR = err_code[7];}	//ошибка отсутствие EOL
			if (LENGHT_INT!= (N_BYTES_RECEIVED-12)) {ERROR = err_code[15];}	//длина не соответсвует LEN
			if (LENGHT_INT&1) {ERROR = err_code[9];}						//нечетное LEN
			BSI_BUF[3] = ERROR; 											//В ответе ERROR на месте BUF[3]
			}
							}						// завершение первой части обработки ошибок
	Bit_Test = 0; //LENGHT_INT = 0;
	MARKER = ON;
}
//.................................................................................
void TRANSFER_CAL_EUSART1(unsigned int LENGHT)			//Работа с БСИ
{
	unsigned int TEMP;
	MARKER = OFF;																		//вот внутренних процедурах скидываем, а по завершении поднимаем
			BSI_BUF[0] = ARRAY_DATA.TYPE_CHAR[1]; BSI_BUF[1] = ARRAY_DATA.TYPE_CHAR[0];	//Здесь уже должен быть подставлен адрес БСИ
			BSI_BUF[2] = CMD>>8; BSI_BUF[3] = CMD;										//BUF[3] обычно ошибки, но в данном случаем я шлю команду БСИ
			TEMP = LENGHT + 8;															//добавляем ADDR и LENGHT
			BSI_BUF[4] = 0; BSI_BUF[5] = 0;												//Длина
			BSI_BUF[7] = LENGHT; LENGHT>>=8; BSI_BUF[6] = LENGHT;																				
			CRC_16 = getCrc16(&BSI_BUF[8], TEMP-8);										//CRC16 считается только по Body 16.09.2016	
			BSI_BUF[TEMP+1] = CRC_16; CRC_16>>=8; BSI_BUF[TEMP] = CRC_16;
			BSI_BUF[TEMP+2] = EOL_LEFT;	BSI_BUF[TEMP+3] = EOL_RIGHT;

			TXSTA1bits.TXEN = ON; ReDe1 = ON; TEMP+=4;

			PIR1bits.TX1IF = ON;
				for (j=0; j<TEMP; j++)	{TXREG1 = BSI_BUF[j];	NOP();	while(PIR1bits.TX1IF == 0);}

			TXSTA1bits.TXEN = OFF;	ReDe1 = OFF;
	MARKER = ON;
}
