#ifndef __ERRORS_H
#define __ERRORS_H

//���� ����� ������

//MEMORY
#define NO_ERRORS									0x00			//������ ���
#define BIG_DATA_WRITE_ERROR						0x11			//������� �������� ������� ������� ����� ������ � ���������� ������
#define SEQUENCE_OF_CMD_DATA_WRITE_ERROR			0x12			//�������� ������������������ ���������� ������ ��� ������ ������ � ���������� ������
#define NOT_ALL_BLOCKS_DATA_RECEIVED_ERROR			0x13			//������� �� ��� ����� ������ ��� ������ � ���������� ������
#define WRONG_BLOCKS_DATA_WRITE_ERROR				0x14			//����� ���������� ������ ������ ��� ������ � ���������� ������ �� ������������� ����������� � ���������� ������
#define WRONG_BLOCKS_DATA_TRANSFER_ERROR			0x15			//�������� ������������������ �������� ������ (� ������� ����������) ��� ������ � ���������� ������
#define CRC16_DATA_SAVED_ERROR						0x16			//������ CRC16 ���������� ������ � ���������� ������
#define WRONG_TRANSFER_LENGHT_DATA_ERROR			0x17			//����� ������������ ������ � ������ ��� ������ � ���������� ������ �� ������������� ��������� ����� �����
//FDISK
#define TRANSFER_FDISK_DATA_ERROR					0x23			//������������ ����� ������ �� FDISK �������� ������ ���� �� � ����� �� �������� � � ���������� ���������
#define BAD_BLOCK_FDISK_ERROR						0x24			//FDISK �� �������� � �������������. ����� ������ ���������
#define FORMAT_FDISK_ERROR							0x25			//��������� �������������� FDISK
#define SECTOR_NOT_EXIST_FDISK_ERROR				0x27			//������������� ������ � FDISK �� ����������
#define FULL_FDISK_ERROR							0x28			//������������� ������ � FDISK �� ����������
//CONTROLLER
#define ANSWER_SLAVE_C0NTROLLER_ERROR				0x31			//��� ������ �� ������������ �����������
#define ANSWER_CDA_ERROR							0x32			//��� ������ �� CDA
#define ANSWER_MEASURE_CHANNEL_ERROR				0x33			//��� ������ �� �������������� ������
//PROTOCOL
#define EOF_ERROR									0x41			//�������������������� ��� EOF ��� ��� ����������, ��������� 4
#define OVER_BUF_LENGHT_ERROR						0x43			//����� ������������ ������ ��������� ������� ������ ������, ��������� 1
#define ODD_BODY_ERROR								0x44			//������ �������� ����� BODY, ��������� 2
#define CRC16_RECEIVED_ERROR						0x45			//������ CRC16 � �������� ������, ��������� 5
#define COMMAND_NOT_SUPPORT_ERROR					0x46			//�������� ��� ������� �� ��������������, ��������� 6
#define ALLOW_EXCUTE_COMMAND_ERROR					0x47			//��� ���������� �� ���������� ������ �������
#define WRONG_PASSWORD_ERROR						0x48			//������������ ������
#define WRITE_DATA_ERROR							0x49			//������ � ���������� ������ ���������
#define REQUEST_DATA_ERROR							0x4B			//������������� ������ ���
#define BODY_NOT_EQUALS_LENGHT_ERROR				0x4D			//����� BODY �� ������������� LEN, ��������� 3
#define COMMAND_FDISK_ERROR							0x4E			//������ ��������� ������� � ������ "�����������" ��� ������������ FDISK
//TECHNOLOGICAL
#define WELL_CLT_INFORMATION_ERROR					0x50			//���������� ������ � ����������� �� �������� � ���������� ������ CLT
#define POLL_CLT_CDA_PARAMETERS_ERROR				0x51			//���������� ���������� ������ ��� CLT ��� CDA, ��������� 7
#define BODY_DATA_PARAMETERS_NOT_REGULATED_ERROR	0x52			//������ � BODY ���������� ������ �� ����������������
#define SET_TIME_ERROR								0x53			//���������� ������� ���� ����������� ��������
#define LOW_VOLTAGE_ERROR							0x54			//�� ����������� �����, ���� ��� ���������
#define POLL_TOOL_LESS_TIME_ERROR					0x56			//����� ������ ����������� ���� �� ��� ������ ���������� ������ �������� �������
#define CYCLE_CDA_TIME_ERROR						0x57			//����� ����� ������ ���� �� ������ CDA ������ �������������� ������� ��������� ���������� ������� CDA
#define CDA_NOT_111_ERROR							0x58			//� ������� ����� ������ CDA ��� ������� ���� �� ����� 111
#define REGISTRATION_NOT_PARAMETERS_ERROR			0x59			//�� ������ ��������������� ��������� �����������
//USERS
#define INCOMPLETE_DATA_ERROR						0xC0			//�������� ������, ���� ���� �� ����������� ��� �� �������

#endif
/*
CDA 		- ������� ����� ������ �� ������������� ������.
CLT			- �������� ���������� (������� ����� ������ �� CDA).
ADDR		- ����������� ����� ����������� ����������.
CMD			- ����������� ��� �������.
BODY		- ������ � ���� ������.
LEN			- ����� ���� ������ � ������.
CRC16		- ����������� ����� �� ��������� CCITT CRC-16, ������� 0x1021 (��� ������������� 0xFFFF).
ERR			- ���� ���� ������.
EOF 		- ����������� ������� ����� ������, ��� 0x00FF.
FDISK		- ������� ���� ������.
������		- ����������, ��������������� ��� ���������� ���, ����������� � �������� ����� � ������� ������� � ������ �������.
������		- ���������� ������������� ��� ����������� ����������, ����������� ������ �������.
*/