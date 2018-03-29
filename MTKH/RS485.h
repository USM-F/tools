#ifndef __RS485_H
#define __RS485_H

#include <definitions.h>

#define ADDR			0xCF	//����� ������� ���������, 0xC8 ����� �������� �������
#define BASE_ADDR		0xFFF4	//��������������� �������, �� ������� ���� �������� ����������� � �������
#define BASE_ADDR_LEFT	0xFF	//����� �����
#define BASE_ADDR_RIGHT	0xF4	//������� �����

#define MASK_BSI		0x0F	//����� ����������� ���

#define ADDR_BSI_TNZ	0x01	//����� ������ ��������� � ������� � ��������
#define ADDR_BSI_ACS	0x02	//����� ������ �������������� � ������� � ��������
#define ADDR_BSI_RB		0x03	//����� ������ �������������� � ������� � ��������
#define ADDR_BSI_TD		0x04	//����� ������ ���������-�������� � ������� � �������� 0x04

#define ADDR_TD			0xCF04	//����� ������ ���������-�������� ���������	RS-485
#define ADDR_RB			0xCF03	//����� ������ �������������� ��������� RS-485

//��� ��� �� RS-485, ���� ��������� �������
#define INF				0x0100	//������ ���������� � �������
#define TRANS			0x0400	//�������� ������ ���������� ���������
#define ECHO 			0x0500	//���-����

#define TR_ID	 		0x2000	//�������� �������������
#define ALLOW_ID 		0x2100	//��������� ������ ��������������
#define WR_ID 			0x2200	//�������� �������������

#define TR_CAL 			0x3000	//�������� ����������
#define ALLOW_CAL 		0x3100	//��������� ������ ����������
#define WR_CAL 			0x3200	//�������� ����������

#define EOL 			0x11FF	//������� ����� ���������

#define EOL_LEFT		0x00	//�������� 16.09.2016 ����� �������� �� d60
#define EOL_RIGHT		0xFF

#define SHIFT_BUF		18		//����� � ������ ��� ���������� ���� ����������

extern unsigned char	ERROR;

extern unsigned char	ADDR_MAIN;				//����� BUF[0]
extern unsigned char	ADDR_ADDITIONAL;		//����� BUF[1] �������������� ����� ���� 0x00 (ALL), 0x01 (TNZ), 0x02 (ACS), 0x03 (REZ), 0x04 (TD)
extern unsigned int		CMD;					//������� BUF[2], BUF[3] 16 ���

extern unsigned int		j;
extern unsigned int		LENGHT_INT;				//����� 16 ��� BUF[6], BUF[7]
extern const rom unsigned char err_code[];			//error codes
extern const rom unsigned char Password[];			//Password

//��������� �������
void RECIEVE_EUSART1(void);							//������ � ������������ ���
void TRANSFER_EUSART1(unsigned int  LENGHT);		
void RETRANSFER_EUSART1(unsigned int LENGHT);		//��� ��������� ��������� ���

void RECIEVE_EUSART2(void);							//������ � ���������
void TRANSFER_EUSART2(unsigned int LENGHT);
void TRANSFER_EUSART2_INV(unsigned int LENGHT);		//�������� ������ rs-485 c ������������ BODY

void ADAPTER_COMMAND_WORK(void);					//��������� ������� ��������
void BSI_COMMAND_WORK(void);						//��������� ������ ���

void INF_(void);									//���������� � �������

void RECIEVE_CAL_EUSART1(void);
void TRANSFER_CAL_EUSART1(unsigned int LENGHT);		//������ � ���

#endif

/*----------------������-------------------------------
[0]  0x11 - ������� ��������� �������������� ������� ������ ��������� ������� ��������
[1]  0x12 - �������� ������������������ ���������� ������ ��� ����������� ������ ��������
[2]  0x13 - ������� �� ��� ����� ��������������
[3]  0x14 - ����� ���������� �������� ������ ������ ��� ����������� �������� 
						�� ������������� ����������� � ���������� ������
[4]  0x15 - �������� ������������������ �������� ������ (� ������� ����������)
[5]  0x16 - ������ CRC16 � ���������� ������ ��������
[6]  0x31 - ��� ����� � ����������� ������������
[7]  0x41 - ���������� EOL
[8]  0x43 - ����� ������ ������ ��������� ������� ������
[9]  0x44 - ������ �������� ����� BODY
[10] 0x45 - ������ CRC16
[11] 0x46 - �������� ��� ������� ����������������
[12] 0x48 - ������������ ������
[13] 0x49 - ������ � ������ ���������
[14] 0x4B - ������� ������ ������������� ������� ���������� ��� ��������������
[15] 0x4D - ����� BODY �� ������������� LEN
[16] 0x52 - ������ BODY ��� ��� ����� �� ���������
[17] 0x61 - ������������ ������ BSI ��� ����������� ����� ������
------------------������-----------------------------*/