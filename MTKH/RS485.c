#include <RS485.h>
#include <CRC.h>
#include <flash.h>

const rom unsigned char err_code[]	=	{0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x31, 0x41, 0x43, 0x44, 0x45, 0x46, 0x48, 0x49, 0x4B, 0x4D, 0x52, 0x61};
const rom unsigned char MNEM[]	=		{'M', 'T', 'K', 'H'};								//���������
const rom unsigned char MADD[]	=		{'N', 'O', 'N', 'E'};								//NONE
const rom unsigned char Password[]	=	{0x50, 0x61, 0x73, 0x73, 0x77, 0x6F, 0x72, 0x64};	//Password

unsigned char	ADDR_MAIN;				//����� BUF[0]
unsigned char	ADDR_ADDITIONAL;		//����� BUF[1] �������������� ����� ���� 0x00 (ALL), 0x01 (TNZ), 0x02 (ACS), 0x03 (REZ), 0x04 (TD)

unsigned char	ERROR;					//���� ������
unsigned char	CLEAR_RCREG;			//��� �������� ������� UART
unsigned int	CMD;					//������� BUF[2], BUF[3] 16 ���
unsigned int	j;						//������� 16 ���
unsigned int	LENGHT_INT;				//����� 16 ��� BUF[6], BUF[7]
unsigned int	N_BYTES_RECEIVED;		//���������� �������� ���� 16 ���, LENGHT_INT + 12
unsigned int	TEMP_INT;
//..................................................EUSART2........................
//.................................................................................
void RECIEVE_EUSART2(void)							//������ � ���������
{
	MARKER = OFF;			//�� ���������� ���������� ���������, � �� ���������� ���������

//������� � ���, ��� �� �������� �� �����, ������� ��� ������� �� �������� ������
	ReDe2 = OFF;	ERROR = 0;				//������ ���� ����� ���������� ������!
	TXSTA2bits.TXEN = OFF;					//������ ��������

	RCSTA2bits.CREN = ON;	//�������� ������
//timer_
	T0CON = 0b00000000;	INTCONbits.TMR0IF = OFF;	//set up timer0 - prescaler 1:2, 16-bit timer, timer OFF (7 bit T0CON)
//timer__

	//����� �������
	for (j=0; j<=BUF_SIZE; j++)
	{
	if (j == BUF_SIZE) {j = 1; ERROR = err_code[8]; goto BREAK_LOOP_RECIEVE_EUSART2;}					//����� ����������
	MARKER = ON;
	while(PIR3bits.RC2IF == OFF) {if (INTCONbits.TMR0IF) goto BREAK_LOOP_RECIEVE_EUSART2;}; BUF[j] = RCREG2;
	MARKER = OFF;
	if (RCSTA2bits.OERR) {CLEAR_RCREG = RCREG2; CLEAR_RCREG = RCREG2; CLEAR_RCREG = RCREG2; RCSTA2bits.CREN = OFF; RCSTA2bits.CREN = ON;}
	if (RCSTA2bits.FERR) {CLEAR_RCREG = RCREG2;}
	TMR0L = TurnTimer0-50;	TMR0H = 0xFF; T0CONbits.TMR0ON = ON;	//������������� ������ �� 73 ��� �� ������������ 
	}

BREAK_LOOP_RECIEVE_EUSART2: INTCONbits.TMR0IF = OFF; N_BYTES_RECEIVED = j; T0CONbits.TMR0ON = OFF;

	CMD = 0; LENGHT_INT = 0; 		//�������� ����� ��������
	RCSTA2bits.CREN = OFF;

	ADDR_MAIN = BUF[0];	ADDR_ADDITIONAL = BUF[1];					//����������� ������ �� ���������� 
	CMD = BUF[2]; CMD<<= 8; CMD+= BUF[3];							//�������� ������� � ������ ������������������
	LENGHT_INT = BUF[6]; LENGHT_INT<<=8; LENGHT_INT+= BUF[7];		//�������� ����� BUF[6], BUF[7] 

	if (ERROR == err_code[8])	{BUF[1] = 0; N_BYTES_RECEIVED = 0; ADDR_ADDITIONAL = 0;}		 		//������ ���������
		else 	{
			Bit_Test =  BUF[N_BYTES_RECEIVED-4]; Bit_Test<<=8; Bit_Test+=BUF[N_BYTES_RECEIVED-3];		// ��� �������� crc
			TEMP_INT = N_BYTES_RECEIVED - 4; CRC_16 = getCrc16(&BUF[8], LENGHT_INT);

	if (Bit_Test != CRC_16) {ERROR = err_code[10];}					//������ CRC
	if ((BUF[N_BYTES_RECEIVED-1] != EOL_RIGHT) || (BUF[N_BYTES_RECEIVED-2] != EOL_LEFT)) {ERROR = err_code[7];}	//������ ���������� EOL
	if (LENGHT_INT!=(N_BYTES_RECEIVED-12)) {ERROR = err_code[15];}	//����� �� ������������ LEN
	if (LENGHT_INT&1) {ERROR = err_code[9];}						//�������� LEN
	BUF[3] = ERROR;													//� ������ ERROR �� ����� BUF[3]
				}

	Bit_Test = 0; //LENGHT_INT = 0;

	MARKER = ON;
}
//.................................................................................
void TRANSFER_EUSART2(unsigned int LENGHT)			//������ � ���������
{
	unsigned int TEMP;
	MARKER = OFF;															//��� ���������� ���������� ���������, � �� ���������� ���������
			BUF[0] = ADDR | 0x7; BUF[1] = ADDR_ADDITIONAL;					//�����
			BUF[2] = 0; BUF[3] = ERROR;										//������
			TEMP = LENGHT + 8;												//��������� ADDR � LENGHT
			BUF[4] = 0; BUF[5] = 0;											//�����
			BUF[7] = LENGHT; LENGHT>>=8; BUF[6] = LENGHT;																				
			CRC_16 = getCrc16(&BUF[8], TEMP-8);								//CRC16 ��������� ������ �� Body 16.09.2016	
			BUF[TEMP+1] = CRC_16; CRC_16>>=8; BUF[TEMP] = CRC_16;
			BUF[TEMP+2] = EOL_LEFT;	BUF[TEMP+3] = EOL_RIGHT;

			TXSTA2bits.TXEN = ON; ReDe2 = ON; TEMP+= 4;
			PIR3bits.TX2IF = ON;
				for (j=0; j<TEMP; j++)	{TXREG2 = BUF[j];	NOP();	while(PIR3bits.TX2IF == 0);}

			TXSTA2bits.TXEN = OFF;	ReDe2 = OFF;
	MARKER = ON;
}
//....................................................................................
void TRANSFER_EUSART2_INV(unsigned int LENGHT)		//�������� ������ rs-485 c ������������� ������� � body
{
	unsigned int TEMP;
	unsigned char Rev1, Rev2;
	MARKER = OFF;															//��� ���������� ���������� ���������, � �� ���������� ���������
			BUF[0] = ADDR | 0x7; BUF[1] = ADDR_ADDITIONAL;					//�����
			BUF[2] = 0; BUF[3] = ERROR;										//������
			TEMP = LENGHT + 8;												//��������� ADDR � LENGHT
			BUF[4] = 0; BUF[5] = 0;											//�����
			BUF[7] = LENGHT; LENGHT>>=8; BUF[6] = LENGHT;
			for (j=8; j<TEMP; j+=2) {Rev1 = BUF[j]; Rev2 = BUF[j+1]; BUF[j] = Rev2; BUF[j+1] = Rev1;}	//������ body
			CRC_16 = getCrc16(&BUF[8], TEMP-8);								//CRC16 ��������� ������ �� Body 16.09.2016	
			BUF[TEMP+1] = CRC_16; CRC_16>>=8; BUF[TEMP] = CRC_16;
			BUF[TEMP+2] = EOL_LEFT;	BUF[TEMP+3] = EOL_RIGHT;

			TXSTA2bits.TXEN = ON; ReDe2 = ON; TEMP+= 4;
			PIR3bits.TX2IF = ON;
				for (j=0; j<TEMP; j++)	{TXREG2 = BUF[j];	NOP();	while(PIR3bits.TX2IF == 0);}

			TXSTA2bits.TXEN = OFF;	ReDe2 = OFF;
	MARKER = ON;

	TEMP-=4; for (j=8; j<TEMP; j+=2) {Rev2 = BUF[j+1]; Rev1 = BUF[j]; BUF[j] = Rev2; BUF[j+1] = Rev1;}	//���������� �� �����
}
//..................................................EUSART1........................
//.................................................................................
void RECIEVE_EUSART1(void)							//������ � ���
{
	MARKER = OFF;	//�� ���������� ���������� ���������, � �� ���������� ���������

//	CMD = 0;
	LENGHT_INT = 0; ERROR = 0;				//�������� �������� ���������
	ReDe1 = OFF;									//������ ���� ����� ���������� ������!
	TXSTA1bits.TXEN = OFF; 							//������ ��������
//timer_
	TMR0H = 0xE2; TMR0L = 0xB3;						//5ms �� ������������ �� �������
	if ((CMD == WR_CAL) || (CMD == WR_ID)) {TMR0H = 0x00; TMR0L = 0x01;}//45ms �� ������ ���������� (��������������), ��� ��� ����� ����������(�������������), ��������� � ������
	CMD = 0;										//�������� ����� ���� ��� ���������� ����� �������� ������
//	TMR0H = 0xA8; TMR0L = 0x1B;						//15ms �� ������������ �� �������, ��� ������ ��������������
//	TMR0H = 0xE2; TMR0L = 0xB3;						//5ms �� ������������ �� �������
	T0CON = 0b10000000;	INTCONbits.TMR0IF = OFF;	//set up timer0 - prescaler 1:2, 16-bit timer, timer ON (7 bit T0CON)
//timer__
	RCSTA1bits.CREN = ON;	//�������� ������
	//����� ������
	while(PIR1bits.RC1IF == OFF) {if (INTCONbits.TMR0IF)	{ERROR = err_code[6];	goto BREAK_LOOP_RECIEVE_EUSART1;}}; BUF[0] = RCREG1;	//������ �� ��������
	TMR0L = TurnTimer0-130;	TMR0H = 0xFF;	//������� ������ ����, ����������� ������ �� 126 mcs

	for (j=1; j<=BUF_SIZE; j++)
	{
	if (j == BUF_SIZE) {j = 1; ERROR = err_code[8]; goto BREAK_LOOP_RECIEVE_EUSART1;}

	while(PIR1bits.RC1IF == OFF) {if (INTCONbits.TMR0IF) goto BREAK_LOOP_RECIEVE_EUSART1;}; BUF[j] = RCREG1;
	if (RCSTA1bits.OERR) {CLEAR_RCREG = RCREG1; CLEAR_RCREG = RCREG1; CLEAR_RCREG = RCREG1; RCSTA1bits.CREN = OFF; RCSTA1bits.CREN = ON;}

	TMR0L = TurnTimer0-30;	TMR0H = 0xFF;	//������� ������ ����, ����������� ������ �� 60 mcs
	}

BREAK_LOOP_RECIEVE_EUSART1: INTCONbits.TMR0IF = OFF; N_BYTES_RECEIVED = j; T0CONbits.TMR0ON = OFF;

	RCSTA1bits.CREN = OFF;

	ADDR_MAIN = BUF[0];	ADDR_ADDITIONAL = BUF[1];					//����������� ������ �� ���������� 
	CMD = BUF[2]; CMD<<= 8; CMD+= BUF[3];							//�������� ������� � ������ ������������������
	LENGHT_INT = BUF[6]; LENGHT_INT<<=8; LENGHT_INT+= BUF[7];		//�������� ����� BUF[6], BUF[7] 

	if (ERROR!= err_code[6]) {						// 0x31 - ��� ����� � ����������� ������������

	if (ERROR == err_code[8])	{BUF[1] = 0; N_BYTES_RECEIVED = 0; ADDR_ADDITIONAL = 0;}		 //������ ���������
		else {
			Bit_Test = BUF[N_BYTES_RECEIVED-4]; Bit_Test<<= 8; Bit_Test+= BUF[N_BYTES_RECEIVED-3];		// ��� �������� crc
			TEMP_INT = N_BYTES_RECEIVED - 4; CRC_16 = getCrc16(&BUF[8], LENGHT_INT);

			if (Bit_Test != CRC_16) {ERROR = err_code[10];}					//������ CRC
			if ((BUF[N_BYTES_RECEIVED-1] != EOL_RIGHT) || (BUF[N_BYTES_RECEIVED-2] != EOL_LEFT)) {ERROR = err_code[7];}	//������ ���������� EOL
			if (LENGHT_INT!= (N_BYTES_RECEIVED-12)) {ERROR = err_code[15];}	//����� �� ������������ LEN
			if (LENGHT_INT&1) {ERROR = err_code[9];}						//�������� LEN
			BUF[3] = ERROR; 												//� ������ ERROR �� ����� BUF[3]
			}
							}						// ���������� ������ ����� ��������� ������
	Bit_Test = 0; //LENGHT_INT = 0;
	MARKER = ON;
}
//.................................................................................
void TRANSFER_EUSART1(unsigned int LENGHT)			//������ � ���
{
	unsigned int TEMP;
	MARKER = OFF;																//��� ���������� ���������� ���������, � �� ���������� ���������
			BUF[0] = ARRAY_DATA.TYPE_CHAR[1]; BUF[1] = ARRAY_DATA.TYPE_CHAR[0];	//����� ��� ������ ���� ���������� ����� ���
			BUF[2] = 0; BUF[3] = 0;												//BUF[3] ������ ������, �� � ������ ������� � ��� ������� ���
			TEMP = LENGHT + 8;													//��������� ADDR � LENGHT
			BUF[4] = 0; BUF[5] = 0;												//�����
			BUF[7] = LENGHT; LENGHT>>=8; BUF[6] = LENGHT;																				
			CRC_16 = getCrc16(&BUF[8], TEMP-8);									//CRC16 ��������� ������ �� Body 16.09.2016	
			BUF[TEMP+1] = CRC_16; CRC_16>>=8; BUF[TEMP] = CRC_16;
			BUF[TEMP+2] = EOL_LEFT;	BUF[TEMP+3] = EOL_RIGHT;

			TXSTA1bits.TXEN = ON; ReDe1 = ON; TEMP+=4;
			PIR1bits.TX1IF = ON;

				for (j=0; j<TEMP; j++)	{TXREG1 = BUF[j];	NOP();	while(PIR1bits.TX1IF == 0);}//���� �� ���������� ����, ���������� ��� ������� ����

			TXSTA1bits.TXEN = OFF;	ReDe1 = OFF;
	MARKER = ON;
}
//.................................................................................
void RETRANSFER_EUSART1(unsigned int LENGHT)			//������ � ���
{
	unsigned int TEMP;
if (CMD == TRANS) {BUF[8] = 0; BUF[9] = 0;}
	MARKER = OFF;																//��� ���������� ���������� ���������, � �� ���������� ���������
			BUF[0] = ARRAY_DATA.TYPE_CHAR[1]; BUF[1] = ARRAY_DATA.TYPE_CHAR[0];	//����� ��� ������ ���� ���������� ����� ���
			BUF[2] = CMD>>8; BUF[3] = CMD;										//BUF[3] ������ ������, �� � ������ ������� � ��� ������� ���
			TEMP = LENGHT + 8;													//��������� ADDR � LENGHT
			BUF[4] = 0; BUF[5] = 0;												//�����
			BUF[7] = LENGHT; LENGHT>>=8; BUF[6] = LENGHT;																				
			CRC_16 = getCrc16(&BUF[8], TEMP-8);									//CRC16 ��������� ������ �� Body 16.09.2016	
			BUF[TEMP+1] = CRC_16; CRC_16>>=8; BUF[TEMP] = CRC_16;
			BUF[TEMP+2] = EOL_LEFT;	BUF[TEMP+3] = EOL_RIGHT;

			TXSTA1bits.TXEN = ON; ReDe1 = ON; TEMP+=4;
			PIR1bits.TX1IF = ON;

				for (j=0; j<TEMP; j++)	{TXREG1 = BUF[j];	NOP();	while(PIR1bits.TX1IF == 0);}//���� �� ���������� ����, ���������� ��� ������� ����

			TXSTA1bits.TXEN = OFF; ReDe1 = OFF;
	MARKER = ON;
}
//..........................WORKING................................................
//.................................................................................
void ADAPTER_COMMAND_WORK(void)					//��������� ������� ��������
{

switch (CMD)	{

		case INF:	INF_();		break;
		case TRANS:	GET_ALL();	break;
		case ECHO:	LENGHT_INT = N_BYTES_RECEIVED - 12; TRANSFER_EUSART2(LENGHT_INT);	break;					//���-����, ������� 12 ����
		case TR_ID:	//��������� �������������
					if	(LENGHT_INT == 0)	{
						if (ADDR_ADDITIONAL == 0x00){				//�������� ������ �� ����������� ��������� 19.01.2015
						ADRESS_1 = 0x5000;
						ReadFlash((UINT32) ADRESS_1, (UINT16) ID_LENGHT, &BUF[8]);
						LENGHT_INT = (unsigned int) ID_LENGHT;	TRANSFER_EUSART2(LENGHT_INT);
													}
											}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;

		case ALLOW_ID:												//��������� ������ ��������������
						for (i=8; i<16;i++)	{						//�������� ������
						FLAG.ID = ON;
						if (BUF[i] != Password[i-8]) {FLAG.ID = OFF; ERROR = err_code[12];	break;}
											}	TRANSFER_EUSART2(0);	break; 
		case WR_ID:	//�������� �������������, 128 ����
					if	(LENGHT_INT == ID_LENGHT)	{
						if (FLAG.ID == ON)	{
							LENGHT_INT = (unsigned int) ID_LENGHT; TRANSFER_EUSART2(LENGHT_INT);					//����� ���, � body ����� �����������
							ADRESS_1 = 0x5000;	ADRESS_2 = ADRESS_1 + (UINT32) ID_LENGHT;
							EraseFlash((UINT32) ADRESS_1,(UINT32)ADRESS_2);
							WriteBytesFlash((UINT32)ADRESS_1,(UINT16) ID_LENGHT, &BUF[8]); FLAG.ID = OFF;
							//ReadFlash((UINT32) ADRESS_1, (UINT16) ID_LENGHT, &BUF[8]);							//�����-������������� ��� ������ ����� ������, �� ��� ��� �����
							//LEN = (unsigned char) ID_LENGHT;	reply(LEN);	
											}	else {ERROR = err_code[13]; TRANSFER_EUSART2(0);}					//������ � ������ ���������
													}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;	//����� BODY �� ���������

		case TR_CAL:	//��������� ����������
						if	(LENGHT_INT == 0)	{
							if (ADDR_ADDITIONAL==0x00)	{//�������� ����������� ��������� ����� ������������ 25.10.2016
							for (j=0;j<33;j++) {BUF[j] = 0;}		//���� ������ ������ � ����� ������������� �������������
							BUF[12] = 0x8E; BUF[13] = 0x02;			//654 ����� ���������� ����� ����� �����. - ����� ����� ����������)
			//���������� ��������������
							GLOBAL_ERROR = 0;
							ERROR = 0; MARKER = OFF; Sstart = SHIFT_BUF;	Eend = Sstart + CAL_ACS_LENGHT - 1;	//SHIFT_BUF ��� ������ ������, ������� �������� 8 ���� + ������������� ������������ MTKH 8 ����, 2 ����� ��� ������
							ADRESS_1 = 0x5300;	ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_ACS_LENGHT, &BUF[Sstart]);				//[16],[17] ERROR, CAL_ACS_LENGHT 122 ����, 18-139 in BUF
							if	(ERROR)	{BUF[Sstart-1] = ERROR; BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR;} else {BUF[Sstart-1] = 0x00; BUF[Sstart-2] = 0x00;}
			//���������� ���������-������
							Sstart = Eend + 3;	Eend = Sstart + CAL_TNZ_LENGHT - 1;
							ADRESS_1 = 0x5500;	ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_TNZ_LENGHT, &BUF[Sstart]);				//[140],[141] ERROR, CAL_TNZ_LENGHT 340 ����, 142-481 in BUF
							if	(ERROR)	{BUF[Sstart-1] = ERROR; BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR;} else {BUF[Sstart-1] = 0x00; BUF[Sstart-2] = 0x00;}
			//���������� ��������������
							Sstart = Eend + 3;	Eend = Sstart + CAL_RB_LENGHT - 1;
							ARRAY_DATA.TYPE_INT = ADDR_RB;  CMD = TR_CAL;	TRANSFER_CAL_EUSART1(0);	RECIEVE_CAL_EUSART1();			//[482],[483] ERROR, CAL_RB_LENGHT 46 ����, 484-529 in BUF
							if	(ERROR)	{BUF[Sstart-1] = ERROR; BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR;} else {BUF[Sstart-1] = 0x00; BUF[Sstart-2] = 0x00;}
							for (j=Sstart;j<=Eend;j++) {BUF[j] = BSI_BUF[j-476];}
			//���������� ����������-���������
							Sstart = Eend + 3;	Eend =  Sstart + CAL_TD_LENGHT - 1;
							ARRAY_DATA.TYPE_INT = ADDR_TD;	 CMD = TR_CAL;	TRANSFER_CAL_EUSART1(0);	RECIEVE_CAL_EUSART1();			//[530],[531] ERROR, CAL_TD_LENGHT 138 ����, 532-669 in BUF
							if	(ERROR)	{BUF[Sstart-1] = ERROR; BUF[Sstart-2] = 0x00; GLOBAL_ERROR = ERROR;} else {BUF[Sstart-1] = 0x00; BUF[Sstart-2] = 0x00;}
							for (j=Sstart;j<=Eend;j++) {BUF[j] = BSI_BUF[j-524];}

							LENGHT_INT = CAL_ALL_LENGHT; ADDR_MAIN = ADDR; ADDR_ADDITIONAL = 0x00;					//���������� ��������� ����� � ������� � �����
							if (GLOBAL_ERROR) {ERROR = INCOMPLETE_DATA_ERROR;} else {ERROR = 0;}								
//ERROR = 0;
							TRANSFER_EUSART2(LENGHT_INT);											//�������� �����
							for (j=0;j<BSI_BUF_SIZE;j++) {BSI_BUF[j] = 0;}
														}
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;		//����� BODY �� ���������

/*		������ ����� ����������, �������� ������ ������
		case TR_CAL:	//��������� ����������
						if	(LENGHT_INT == 0)	{
							if (ADDR_ADDITIONAL==0x00)	{//�������� ������ �� ����������� ��������� 19.01.2015
							ADRESS_1 = 0x5100;
							ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_LENGHT, &BUF[8]);
							LENGHT_INT = (unsigned int) CAL_LENGHT;	TRANSFER_EUSART2(LENGHT_INT);
														}
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;		//����� BODY �� ���������
		case ALLOW_CAL:	//��������� ������
						if	(LENGHT_INT == 0)	{
											TRANSFER_EUSART2(0); FLAG.CAL = ON;
												}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;		//����� BODY �� ���������	
		case WR_CAL:	//�������� ����������, 194 ����
					if	(LENGHT_INT == CAL_LENGHT)	{
						if (FLAG.CAL == ON)	{
							LENGHT_INT = (unsigned int) CAL_LENGHT; TRANSFER_EUSART2_INV(LENGHT_INT);				//������� ��� �������� �������, ����� ���, � body ����� �����������
							ADRESS_1 = 0x5100;	ADRESS_2 = ADRESS_1 + (UINT32) CAL_LENGHT;
							EraseFlash((UINT32) ADRESS_1 ,(UINT32) ADRESS_2);
							WriteBytesFlash((UINT32) ADRESS_1,(UINT16) CAL_LENGHT, &BUF[8]); FLAG.CAL = OFF; 
							//ReadFlash((UINT32) ADRESS_1, (UINT16) CAL_LENGHT, &BUF[8]);
							//LEN = (unsigned char) CAL_LENGHT;	reply(LEN);											//�����-���������� ��� ������ ����� ������, �� ��� ��� �����
											}	else {ERROR = err_code[13]; TRANSFER_EUSART2(0);}					//������ � ������ ���������
													}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	break;	//����� BODY �� ���������
*///������ ����� ����������
		default:	ERROR = err_code[11]; TRANSFER_EUSART2(0); break;	//�������� ��� ������� ����������������
				}	
	for (j=0;j<N_BYTES_RECEIVED;j++) {BUF[j] = 0;}	//������� ������
}	//����� ��������� ADAPTER_COMMAND_WORK
//.................................................................................
void BSI_COMMAND_WORK(void)		//������ � ��� �� ������� USART	
{
//	TRANSFER_EUSART2(LENGHT_INT);
	RETRANSFER_EUSART1(LENGHT_INT);	RECIEVE_EUSART1();
	
	if (ERROR == err_code[6]) {TRANSFER_EUSART2(0); return;}	

		switch (ARRAY_DATA.TYPE_INT)	{

			case ADDR_RB:		ADDR_ADDITIONAL =  ADDR_BSI_RB;		 TRANSFER_EUSART2(LENGHT_INT);	break;
			case ADDR_TD:		ADDR_ADDITIONAL = ADDR_BSI_TD;		 TRANSFER_EUSART2(LENGHT_INT);	break;

			default:	ERROR = err_code[11]; TRANSFER_EUSART2(0); break;	//�������� ��� ������� ����������������
										}

}	//����� ��������� BSI_COMMAND_WORK

//....................................................................................
void INF_(void)							//������� �� �������� � ������� ���������� � �������
{		
if (LENGHT_INT == 0)	{
				for (j=8; j<12; j++)	{BUF[j] = MNEM[j-8];}
				for (j=12; j<16; j++)	{BUF[j] = MADD[j-12];}
				ReadFlash((UINT32)0x5032,(UINT16) 2, &BUF[16]);					//����� �������� �� ������� �� 50 ���� �� ������ �������
				BUF[18] = ADDR; BUF[19] = 0x00; 								//����� �������
				BUF[20] = 0x00; BUF[21] = 0x0F; 								//����� ����������� ���
				LENGHT_INT = 14;	TRANSFER_EUSART2(LENGHT_INT);
						}	else {ERROR = err_code[16]; TRANSFER_EUSART2(0);}	//����� BODY �� ���������
}
//.....................................................................................
void RECIEVE_CAL_EUSART1(void)							//������ ������� ���������� �� ������������ ���
{
	MARKER = OFF;	//�� ���������� ���������� ���������, � �� ���������� ���������

	//CMD = 0; 
	LENGHT_INT = 0; ERROR = 0;				//�������� �������� ���������
	ReDe1 = OFF;									//������ ���� ����� ���������� ������!
	TXSTA1bits.TXEN = OFF; 							//������ ��������
//timer_
	TMR0H = 0xE2; TMR0L = 0xB3;						//5ms �� ������������ �� �������
	if ((CMD == WR_CAL) || (CMD == WR_ID)) {TMR0H = 0x00; TMR0L = 0x01;}//45ms �� ������ ���������� (��������������), ��� ��� ����� ����������(�������������), ��������� � ������
	CMD = 0;										//�������� ����� ���� ��� ���������� ����� �������� ������
//	TMR0H = 0xA8; TMR0L = 0x1B;						//15ms �� ������������ �� �������, ��� ������ ��������������
//	TMR0H = 0xE2; TMR0L = 0xB3;						//5ms �� ������������ �� �������
	T0CON = 0b10000000;	INTCONbits.TMR0IF = OFF;	//set up timer0 - prescaler 1:2, 16-bit timer, timer ON (7 bit T0CON)
//timer__
	RCSTA1bits.CREN = ON;	//�������� ������

	//����� ������
	while(PIR1bits.RC1IF == OFF) {if (INTCONbits.TMR0IF)	{ERROR = err_code[6];	goto BREAK_LOOP_RECIEVE_CAL_EUSART1;}}; BSI_BUF[0] = RCREG1;	//������ �� ��������
	TMR0L = TurnTimer0-130;	TMR0H = 0xFF;	//������� ������ ����, ����������� ������ �� 126 mcs

	for (j=1; j<=BSI_BUF_SIZE; j++)
	{
	if (j == BSI_BUF_SIZE) {j = 1; ERROR = err_code[8]; goto BREAK_LOOP_RECIEVE_CAL_EUSART1;}

	while(PIR1bits.RC1IF == OFF) {if (INTCONbits.TMR0IF) goto BREAK_LOOP_RECIEVE_CAL_EUSART1;}; BSI_BUF[j] = RCREG1;
	if (RCSTA1bits.OERR) {CLEAR_RCREG = RCREG1; CLEAR_RCREG = RCREG1; CLEAR_RCREG = RCREG1; RCSTA1bits.CREN = OFF; RCSTA1bits.CREN = ON;}

	TMR0L = TurnTimer0-30;	TMR0H = 0xFF;	//������� ������ ����, ����������� ������ �� 60 mcs
	}

BREAK_LOOP_RECIEVE_CAL_EUSART1: INTCONbits.TMR0IF = OFF; N_BYTES_RECEIVED = j; T0CONbits.TMR0ON = OFF;

	RCSTA1bits.CREN = OFF;

	ADDR_MAIN = BSI_BUF[0];	ADDR_ADDITIONAL = BSI_BUF[1];					//����������� ������ �� ���������� 
	CMD = BSI_BUF[2]; CMD<<= 8; CMD+= BSI_BUF[3];							//�������� ������� � ������ ������������������
	LENGHT_INT = BSI_BUF[6]; LENGHT_INT<<=8; LENGHT_INT+= BSI_BUF[7];		//�������� ����� BUF[6], BUF[7] 

	if (ERROR!= err_code[6]) {						// 0x31 - ��� ����� � ����������� ������������

	if (ERROR == err_code[8])	{BSI_BUF[1] = 0; N_BYTES_RECEIVED = 0; ADDR_ADDITIONAL = 0;}		 //������ ���������
		else {
			Bit_Test = BSI_BUF[N_BYTES_RECEIVED-4]; Bit_Test<<= 8; Bit_Test+= BSI_BUF[N_BYTES_RECEIVED-3];		// ��� �������� crc
			TEMP_INT = N_BYTES_RECEIVED - 4; CRC_16 = getCrc16(&BSI_BUF[8], LENGHT_INT);

			if (Bit_Test != CRC_16) {ERROR = err_code[10];}					//������ CRC
			if ((BSI_BUF[N_BYTES_RECEIVED-1] != EOL_RIGHT) || (BSI_BUF[N_BYTES_RECEIVED-2] != EOL_LEFT)) {ERROR = err_code[7];}	//������ ���������� EOL
			if (LENGHT_INT!= (N_BYTES_RECEIVED-12)) {ERROR = err_code[15];}	//����� �� ������������ LEN
			if (LENGHT_INT&1) {ERROR = err_code[9];}						//�������� LEN
			BSI_BUF[3] = ERROR; 											//� ������ ERROR �� ����� BUF[3]
			}
							}						// ���������� ������ ����� ��������� ������
	Bit_Test = 0; //LENGHT_INT = 0;
	MARKER = ON;
}
//.................................................................................
void TRANSFER_CAL_EUSART1(unsigned int LENGHT)			//������ � ���
{
	unsigned int TEMP;
	MARKER = OFF;																		//��� ���������� ���������� ���������, � �� ���������� ���������
			BSI_BUF[0] = ARRAY_DATA.TYPE_CHAR[1]; BSI_BUF[1] = ARRAY_DATA.TYPE_CHAR[0];	//����� ��� ������ ���� ���������� ����� ���
			BSI_BUF[2] = CMD>>8; BSI_BUF[3] = CMD;										//BUF[3] ������ ������, �� � ������ ������� � ��� ������� ���
			TEMP = LENGHT + 8;															//��������� ADDR � LENGHT
			BSI_BUF[4] = 0; BSI_BUF[5] = 0;												//�����
			BSI_BUF[7] = LENGHT; LENGHT>>=8; BSI_BUF[6] = LENGHT;																				
			CRC_16 = getCrc16(&BSI_BUF[8], TEMP-8);										//CRC16 ��������� ������ �� Body 16.09.2016	
			BSI_BUF[TEMP+1] = CRC_16; CRC_16>>=8; BSI_BUF[TEMP] = CRC_16;
			BSI_BUF[TEMP+2] = EOL_LEFT;	BSI_BUF[TEMP+3] = EOL_RIGHT;

			TXSTA1bits.TXEN = ON; ReDe1 = ON; TEMP+=4;

			PIR1bits.TX1IF = ON;
				for (j=0; j<TEMP; j++)	{TXREG1 = BSI_BUF[j];	NOP();	while(PIR1bits.TX1IF == 0);}

			TXSTA1bits.TXEN = OFF;	ReDe1 = OFF;
	MARKER = ON;
}
