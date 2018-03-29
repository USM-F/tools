#ifndef __definitions_H
#define __definitions_H

#include <p18f25k80.h>
#include <delays.h>
#include <Errors.h>

//кварц на 12 MHz, 5 Вольт
#define _XTAL_FREQ	12000000L
#define	BAUDRATE	375000
#define NOP() {_asm nop _endasm}

#define ON		1
#define OFF		0

//команды по манчестеру
#define	TOOL			0x0000	//адрес прибора, общий
#define	TOOL_TD			0xC000	//адрес для работы с темрометром-манометром
#define	TOOL_RB			0x8000	//адрес для работы с резистивиметром

#define	VERSION			0x0001	// версия прошивки

#define ID_LENGHT		128		//длина идентификатора, у всех одинаковая

#define CAL_ACS_LENGHT	122		//длина калибровки резистивиметра
#define CAL_TNZ_LENGHT	340		//длина калибровки натяжения-сжатия, было 162 (до 15.11.2016)
#define CAL_RB_LENGHT	46		//длина калибровки резистивиметра
#define CAL_TD_LENGHT	138		//длина калибровки термометра-манометра
#define CAL_ALL_LENGHT	662		//длина калибровки по адресу CF00, был 484 (до 15.11.2016)

//команды манчестера
#define Tool_Tst		0x0500	//запрос теста
#define Tool_Tnz_Tst	0xCD00	//тест TNZ PIC
#define Tool_Tnz_Tst	0xCD00	//тест TNZ PIC

#define Tool_Tnz 		0xC910	//передать данные TNZ
#define Tool_An  		0xD910	//передать данные ACS
#define Tool_ALL_TM		0x0710	//передать TNZ и  ACS
#define Tool_ALL		0x0730	//передать TNZ,ACS,TD,RB

#define	Transmit_ID		0x0502	//передать ИЗ регистратору
#define	Transmit_CAL	0x0504	//передать КЗ регистратору
#define	Modific_CAL		0x0581	//разрешить запись КЗ в Flash
#define	Modific_ID		0x05C9	//разрешить запись ИЗ в Flash
#define	Write_ID_CAL	0x0600	//запись байта в КЗ или ИЗ в Flash
#define	Repeat_ID_CAL	0x0582	//разрешить повторную запись байта в КЗ или ИЗ в Flash
#define	Test_Line		0x0500	//тест линии связи

#define	ADDR_MASK		0xF800	//маскирует 5 бит в слове, кот. являются адресом прибора
#define	ADDIT_MASK		0x07FF	//маскирует 11 бит в слове, команда
#define	WRITE_MASK		0x0700	//при записи байта

#define Tool_En			0x0108;	//en UART
#define Tool_Dis		0x0101;	//dis UART

#define TNUM			0x0001	//номер прибора

#define	Acus			0x18	// адрес акустики, надо игнорировать долго

//параметры буфера, адрес во flash и др.
#define START_ADDR_ID	0x5000	//стартовый адрес для записи идентификатора
#define START_ADDR_CAL	0x6000	//стартовый адрес для записи калибровки

#define	LENGHT_AREA		0x0200	//размер области во flash для храниния калибровки или идентификатора

#define BUF_SIZE		1152	//размер буфера
#define BSI_BUF_SIZE	250		//Буфер для размещения данных измерений и передачи калибровки

#define ACS_DATA_SIZE	38		//размер данных ACS
#define TNZ_DATA_SIZE	10		//размер данных TNZ
#define RB_DATA_SIZE	14		//размер данных RB
#define TD_DATA_SIZE	8		//размер данных TD
#define ALL_DATA_SIZE	78		//размер всех данных измерений 78 байт без обвязки


#define RXSL    PORTAbits.RA0	//Программный USART для tnz
#define TXSL	PORTAbits.RA1	//Программный USART для tnz
#define	MARKER	PORTAbits.RA5	//сопровождение декодирования

#define	RXAC	PORTBbits.RB0	//Програмный USART блока акселерометров
#define TXAC    PORTBbits.RB1	//Програмный USART блока акселерометров
#define RS485   PORTBbits.RB2	//Идентификатор протокола приема

//для работы с АДАПТЕРОМ
#define	ReDe2	PORTBbits.RB5	//rs485
#define	DI2		PORTBbits.RB6	//rs485
#define	RO2		PORTBbits.RB7	//модемный сигн. для чтения, а при rs485 RO

//для работы с подчиненными БСИ
#define	ReDe1	PORTCbits.RC5	//rs485
#define	DI1		PORTCbits.RC6	//rs485
#define	RO1		PORTCbits.RC7	//модемный сигн. для чтения, а при rs485 RO

#define	SIG		PORTCbits.RC0	//Прием по манчестеру
#define	TD0		PORTCbits.RC2	//Управление ключем фантомки
#define	TD1		PORTCbits.RC3	//Управление ключем фантомки

#define TurnTimer0		196		//196, 40 mcs

extern unsigned char	*BUF;		// с буфером работаем через указатель
extern unsigned char	*BSI_BUF;	// с буфером данных

extern unsigned char	i;
extern unsigned char	GLOBAL_ERROR;
extern unsigned int		START_FLASH_ADDR;

//------------------
extern unsigned int		Bit_Test;
extern unsigned int		ADRESS_1, ADRESS_2;
 
extern unsigned int		Sstart;		//первая ячейка массива
extern unsigned int		Eend;		//последняя ячейка массива

extern union
{
	unsigned int	TYPE_INT;		// слово данных
	unsigned char	TYPE_CHAR[2];	// байты данных
} ARRAY_DATA; 

extern struct
{
unsigned char ID		: 1;
unsigned char CAL		: 1;
unsigned char FLASH_COM	: 1;
}	FLAG;

void INIT(void);
void GET_ALL(void);					//опрос всех
void Delay_Us(unsigned char NUM);	//один такт процессора 0.33(3) мкс. только не меньше 8 мкс!
void Get_An_Tst(void);				//тест акселерометров
void Get_Tnz_Tst(void);				//тест натяжения

#endif
