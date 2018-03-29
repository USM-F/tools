#include <M2.h>
#include <flash.h>

unsigned char	Rd1;
unsigned char	Rd2;
unsigned char	R1;
unsigned char	R2;
unsigned char	SignalH, SignalL;

unsigned int	COMMAND, COMMAND_Before;
unsigned int	Code;

unsigned int	DATA;
unsigned int	counter;		// счетчик записанных байт
//.................................................................................
void Recieve_Command_M2()	//принять команду с поверхности
{
		Decode = 0;

Strt:	MARKER = OFF; 
		Control = 0;	
		while (SIG==OFF) continue;	//ожидание начала последовательности  

loop:	if (SIG==ON) goto loop; MARKER = ON;
		for (i=0; i<15; i++) {NOP(); if (SIG!=OFF) {goto Strt;}}	
							//подтверждение в течении такта
							//принятия 0 по Sig <58 mcS>
		MARKER = OFF;			//маркер
		Read_Bit(); MARKER = OFF; if (Control) {goto Strt;}		
		MARKER = ON;
		for (i=0; i<15; i++) {NOP(); if (SIG==OFF) {goto Strt;}}
							//проверка принятия второго полупериода <58 mcS>
		MARKER = OFF;
	
		Delay_Us(16);		//16mcs

		for (i=0; i<16; i++)	{
		Decode <<= 1;
		Read_Bit(); R2=SIG; MARKER = OFF;
		if (R1<R2) {Decode=Decode | 1;} else {Bit_Test=Bit_Test | 1;}
		Delay_Us(18);			}	//18mcs

		Delay_Us(44); MARKER = OFF;		//44mcs
 		SignalH=Decode>>8; SignalL=Decode;
		COMMAND = Decode;
}
void Read_Bit(void)
{
   	R1 = SIG; MARKER = ON;
	if(SIG != R1) return;
	if(SIG != R1) return;
	if(SIG != R1) return;
	if(SIG != R1) return;
	if(SIG != R1) return;
	if(SIG != R1) return;
	if(SIG != R1) return;
	if(SIG != R1) return;
	if(SIG != R1) return;
	if(SIG != R1) return;
	if(SIG != R1) return;
	if(SIG != R1) return;
	if(SIG != R1) return;
	if(SIG != R1) return;
	if(SIG != R1) return;
	//26 mcs

	Control=0xf0;
}
//.................................................................................
void Transmit_Low(unsigned int	inCode)		//передача по манчестеру
{
	TD1=ON;Delay_Us(64);TD1=OFF;TD0=ON;Delay_Us(64);	//генерация старт бита 63 mcs
	for (i=0; i<16; i++) {
	if (inCode & 0x8000) 
				{TD1=OFF;TD0=ON; Delay_Us(20); 			//20 mcs
				TD0=OFF;TD1=ON; Delay_Us(10);}			//"1"	10mcs
	else 	
				{
				TD0=OFF;TD1=ON; Delay_Us(20);
				TD1=OFF;TD0=ON; Delay_Us(12);}	//"0" 12mcs
	inCode <<=1;	} // конец кодирования, старший бит вперед
		if (R2 & 1) {TD1=OFF;TD0=ON; Delay_Us(18);		//18mcs 
			      TD0=OFF;TD1=ON; Delay_Us(18);}
		else {TD0=OFF;TD1=ON; Delay_Us(18); 
		      TD1=OFF;TD0=ON; Delay_Us(18);}			// бит на нечет
		TD0=OFF;TD1=OFF;

//Delay10TCYx(37);			//10 mcs добавил 17.07.2015, чтоб лучше отсылала калибровку
}

void Check_CRC(void)
{
	Rd1=0;
	Rd2=0;
	for (Rd1=0, Bit_Test=1, i=0; i<16; i++, Bit_Test <<=1) 
			if (Code & Bit_Test) Rd1++;
	else Rd2++;		//в Rd1 - кол-во "1" из Code
	if (Rd1 & 1) Rd2=0; else Rd2=1;
				// в "Rd2" - бит на нечетность

}

//Отсюда работа с флэшь пошла
//=====================================================================================
void Write_Byte_ID_CAL(void)
{	
	Transmit_Low(COMMAND);

//BUF = BIG_BUF;

	BUF[counter] = SignalL;	counter++;
	
	if ((COMMAND_Before == Modific_ID) && (counter >= ID_LENGHT))
		{
		EraseFlash((UINT32) START_ADDR_ID,(UINT32)(START_ADDR_ID + LENGHT_AREA));

		WriteBytesFlash((UINT32) START_ADDR_ID,(UINT16) ID_LENGHT, &BUF[0]);
		counter = 0; FLAG.ID == OFF;
		} 

	if ((COMMAND_Before == Modific_CAL) && (counter >= CAL_LENGHT))
		{
		EraseFlash((UINT32) START_ADDR_CAL,(UINT32) (START_ADDR_CAL + LENGHT_AREA));

		WriteBytesFlash((UINT32) START_ADDR_CAL,(UINT16) CAL_LENGHT, &BUF[0]);
		counter = 0; FLAG.CAL = OFF;
		} 	
}
//чтение калибровки или идентификатора
//=====================================================================================
void Read_ID(void)
{
//BUF = BIG_BUF;
	ReadFlash((UINT32)START_ADDR_ID,(UINT16) ID_LENGHT, &BUF[0]);	//считывание идентификатора

	Transmit_Low(TOOL);				// адрес прибора
	Transmit_Low(ID_LENGHT);		// длина идентификационной записи, байт = 128

for (j=0;j<ID_LENGHT;j+=2)
	{
	DATA=BUF[j+1]; DATA<<=8; DATA=DATA | BUF[j];
	Transmit_Low(DATA);
	}
}
//--------------------------------------------------------------------------------------
void Read_CAL(void)
{
//BUF = BIG_BUF;
	ReadFlash((UINT32)START_ADDR_CAL,(UINT16) CAL_LENGHT, &BUF[0]);	//считывание калибровки

	Transmit_Low(TOOL);				// адрес прибора
	Transmit_Low(CAL_LENGHT);		// длина калибровочной записи, байт = 278

for (j=0;j<CAL_LENGHT;j+=2)
	{
	DATA=BUF[j+1]; DATA<<=8; DATA=DATA | BUF[j];
	Transmit_Low(DATA);	
	}
}
//--------------------------------------------------------------------------------------
//тест линии связи
void TEST(void)	
{
unsigned char   BUF1[2];

	Transmit_Low(TOOL);	  Transmit_Low(0x0020);	//0x001C
	Transmit_Low(0xC0AA); Transmit_Low(0x3F55);
	Transmit_Low(0xF731); Transmit_Low(0x8000);
	Transmit_Low(0x0000); Transmit_Low(0xFFFF);
	Transmit_Low(0x08CE);
	Transmit_Low(TOOL + (TOOL >> 8));	//10-е слово

	ReadFlash((UINT32)(START_ADDR_ID+50),(UINT16) 2, &BUF1[0]);	//номер хранится со сдвигом на 50 байт от начала таблицы
	DATA=BUF1[1]; DATA<<=8; DATA=DATA | BUF1[0];
	Transmit_Low(DATA);	

	ReadFlash((UINT32)(START_ADDR_ID+53),(UINT16) 2, &BUF1[0]);	//номер программной версии (1 байт) хранится со сдвигом на 53 байт
	DATA=0;	DATA=BUF1[0];
	Transmit_Low(DATA);	

	ReadFlash((UINT32)(START_ADDR_ID+32),(UINT16) 2, &BUF1[0]);	//мнемоника хранится со сдвигом на 32 байт
	DATA=BUF1[1]; DATA<<=8; DATA=DATA | BUF1[0];
	Transmit_Low(DATA);	

	ReadFlash((UINT32)(START_ADDR_ID+34),(UINT16) 2, &BUF1[0]);	
	DATA=BUF1[1]; DATA<<=8; DATA=DATA | BUF1[0];
	Transmit_Low(DATA);	

	ReadFlash((UINT32)(START_ADDR_ID+36),(UINT16) 2, &BUF1[0]);	//доп. мнемоника хранится со сдвигом на 36 байт
	DATA=BUF1[1]; DATA<<=8; DATA=DATA | BUF1[0];
	Transmit_Low(DATA);	

	ReadFlash((UINT32)(START_ADDR_ID+38),(UINT16) 2, &BUF1[0]);	
	DATA=BUF1[1]; DATA<<=8; DATA=DATA | BUF1[0];
	Transmit_Low(DATA);	
}