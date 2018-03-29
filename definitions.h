#ifndef __definitions_H
#define __definitions_H

#include <p18f25k80.h>
#include <delays.h>
#include <Errors.h>

//����� �� 12 MHz, 5 �����
#define _XTAL_FREQ	12000000L
#define	BAUDRATE	375000
#define NOP() {_asm nop _endasm}

#define ON		1
#define OFF		0

//������� �� ����������
#define	TOOL			0x0000	//����� �������, �����
#define	TOOL_TD			0xC000	//����� ��� ������ � �����������-����������
#define	TOOL_RB			0x8000	//����� ��� ������ � ���������������

#define	VERSION			0x0001	// ������ ��������

#define ID_LENGHT		128		//����� ��������������, � ���� ����������

#define CAL_ACS_LENGHT	122		//����� ���������� ��������������
#define CAL_TNZ_LENGHT	340		//����� ���������� ���������-������, ���� 162 (�� 15.11.2016)
#define CAL_RB_LENGHT	46		//����� ���������� ��������������
#define CAL_TD_LENGHT	138		//����� ���������� ����������-���������
#define CAL_ALL_LENGHT	662		//����� ���������� �� ������ CF00, ��� 484 (�� 15.11.2016)

//������� ����������
#define Tool_Tst		0x0500	//������ �����
#define Tool_Tnz_Tst	0xCD00	//���� TNZ PIC
#define Tool_Tnz_Tst	0xCD00	//���� TNZ PIC

#define Tool_Tnz 		0xC910	//�������� ������ TNZ
#define Tool_An  		0xD910	//�������� ������ ACS
#define Tool_ALL_TM		0x0710	//�������� TNZ �  ACS
#define Tool_ALL		0x0730	//�������� TNZ,ACS,TD,RB

#define	Transmit_ID		0x0502	//�������� �� ������������
#define	Transmit_CAL	0x0504	//�������� �� ������������
#define	Modific_CAL		0x0581	//��������� ������ �� � Flash
#define	Modific_ID		0x05C9	//��������� ������ �� � Flash
#define	Write_ID_CAL	0x0600	//������ ����� � �� ��� �� � Flash
#define	Repeat_ID_CAL	0x0582	//��������� ��������� ������ ����� � �� ��� �� � Flash
#define	Test_Line		0x0500	//���� ����� �����

#define	ADDR_MASK		0xF800	//��������� 5 ��� � �����, ���. �������� ������� �������
#define	ADDIT_MASK		0x07FF	//��������� 11 ��� � �����, �������
#define	WRITE_MASK		0x0700	//��� ������ �����

#define Tool_En			0x0108;	//en UART
#define Tool_Dis		0x0101;	//dis UART

#define TNUM			0x0001	//����� �������

#define	Acus			0x18	// ����� ��������, ���� ������������ �����

//��������� ������, ����� �� flash � ��.
#define START_ADDR_ID	0x5000	//��������� ����� ��� ������ ��������������
#define START_ADDR_CAL	0x6000	//��������� ����� ��� ������ ����������

#define	LENGHT_AREA		0x0200	//������ ������� �� flash ��� �������� ���������� ��� ��������������

#define BUF_SIZE		1152	//������ ������
#define BSI_BUF_SIZE	250		//����� ��� ���������� ������ ��������� � �������� ����������

#define ACS_DATA_SIZE	38		//������ ������ ACS
#define TNZ_DATA_SIZE	10		//������ ������ TNZ
#define RB_DATA_SIZE	14		//������ ������ RB
#define TD_DATA_SIZE	8		//������ ������ TD
#define ALL_DATA_SIZE	78		//������ ���� ������ ��������� 78 ���� ��� �������


#define RXSL    PORTAbits.RA0	//����������� USART ��� tnz
#define TXSL	PORTAbits.RA1	//����������� USART ��� tnz
#define	MARKER	PORTAbits.RA5	//������������� �������������

#define	RXAC	PORTBbits.RB0	//���������� USART ����� ��������������
#define TXAC    PORTBbits.RB1	//���������� USART ����� ��������������
#define RS485   PORTBbits.RB2	//������������� ��������� ������

//��� ������ � ���������
#define	ReDe2	PORTBbits.RB5	//rs485
#define	DI2		PORTBbits.RB6	//rs485
#define	RO2		PORTBbits.RB7	//�������� ����. ��� ������, � ��� rs485 RO

//��� ������ � ������������ ���
#define	ReDe1	PORTCbits.RC5	//rs485
#define	DI1		PORTCbits.RC6	//rs485
#define	RO1		PORTCbits.RC7	//�������� ����. ��� ������, � ��� rs485 RO

#define	SIG		PORTCbits.RC0	//����� �� ����������
#define	TD0		PORTCbits.RC2	//���������� ������ ��������
#define	TD1		PORTCbits.RC3	//���������� ������ ��������

#define TurnTimer0		196		//196, 40 mcs

extern unsigned char	*BUF;		// � ������� �������� ����� ���������
extern unsigned char	*BSI_BUF;	// � ������� ������

extern unsigned char	i;
extern unsigned char	GLOBAL_ERROR;
extern unsigned int		START_FLASH_ADDR;

//------------------
extern unsigned int		Bit_Test;
extern unsigned int		ADRESS_1, ADRESS_2;
 
extern unsigned int		Sstart;		//������ ������ �������
extern unsigned int		Eend;		//��������� ������ �������

extern union
{
	unsigned int	TYPE_INT;		// ����� ������
	unsigned char	TYPE_CHAR[2];	// ����� ������
} ARRAY_DATA; 

extern struct
{
unsigned char ID		: 1;
unsigned char CAL		: 1;
unsigned char FLASH_COM	: 1;
}	FLAG;

void INIT(void);
void GET_ALL(void);					//����� ����
void Delay_Us(unsigned char NUM);	//���� ���� ���������� 0.33(3) ���. ������ �� ������ 8 ���!
void Get_An_Tst(void);				//���� ��������������
void Get_Tnz_Tst(void);				//���� ���������

#endif
