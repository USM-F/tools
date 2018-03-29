#ifndef __RS485_H
#define __RS485_H

#include <definitions.h>

#define ADDR			0xCF	//адрес прибора полностью, 0xC8 адрес адаптера прибора
#define BASE_ADDR		0xFFF4	//унифицированная команда, на которую надо ответить информацией о приборе
#define BASE_ADDR_LEFT	0xFF	//Левая часть
#define BASE_ADDR_RIGHT	0xF4	//Прававя часть

#define MASK_BSI		0x0F	//Маска подчиненных бси

#define ADDR_BSI_TNZ	0x01	//адрес модуля натяжения в команде с адаптера
#define ADDR_BSI_ACS	0x02	//адрес модуля акселерометров в команде с адаптера
#define ADDR_BSI_RB		0x03	//адрес модуля резистивиметра в команде с адаптера
#define ADDR_BSI_TD		0x04	//адрес модуля термометр-манометр в команде с адаптера 0x04

#define ADDR_TD			0xCF04	//адрес модуля термометр-манометр настоящий	RS-485
#define ADDR_RB			0xCF03	//адрес модуля резистивиметра настоящий RS-485

//это все по RS-485, этот процессор ведущий
#define INF				0x0100	//Запрос информации о приборе
#define TRANS			0x0400	//Передать данные последнего измерения
#define ECHO 			0x0500	//Эхо-тест

#define TR_ID	 		0x2000	//Передать идентификатор
#define ALLOW_ID 		0x2100	//Разрешить запись идентификатора
#define WR_ID 			0x2200	//Записать идентификатор

#define TR_CAL 			0x3000	//Передать калибровку
#define ALLOW_CAL 		0x3100	//Разрешить запись калибровки
#define WR_CAL 			0x3200	//Записать калибровку

#define EOL 			0x11FF	//Признак конца сообщения

#define EOL_LEFT		0x00	//изменено 16.09.2016 чтобы отличать от d60
#define EOL_RIGHT		0xFF

#define SHIFT_BUF		18		//Сдвиг в буфере для считывания всех калибровок

extern unsigned char	ERROR;

extern unsigned char	ADDR_MAIN;				//Адрес BUF[0]
extern unsigned char	ADDR_ADDITIONAL;		//Адрес BUF[1] Дополнительный может быть 0x00 (ALL), 0x01 (TNZ), 0x02 (ACS), 0x03 (REZ), 0x04 (TD)
extern unsigned int		CMD;					//Команда BUF[2], BUF[3] 16 бит

extern unsigned int		j;
extern unsigned int		LENGHT_INT;				//Длина 16 бит BUF[6], BUF[7]
extern const rom unsigned char err_code[];			//error codes
extern const rom unsigned char Password[];			//Password

//прототипы функций
void RECIEVE_EUSART1(void);							//Работа с подчиненными БСИ
void TRANSFER_EUSART1(unsigned int  LENGHT);		
void RETRANSFER_EUSART1(unsigned int LENGHT);		//Для пересылки сообщения БСИ

void RECIEVE_EUSART2(void);							//Работа с адаптером
void TRANSFER_EUSART2(unsigned int LENGHT);
void TRANSFER_EUSART2_INV(unsigned int LENGHT);		//передача данных rs-485 c перевернутым BODY

void ADAPTER_COMMAND_WORK(void);					//Обработка команды Адаптера
void BSI_COMMAND_WORK(void);						//Обработка команд БСИ

void INF_(void);									//Информация о приборе

void RECIEVE_CAL_EUSART1(void);
void TRANSFER_CAL_EUSART1(unsigned int LENGHT);		//Работа с БСИ

#endif

/*----------------ОШИБКИ-------------------------------
[0]  0x11 - Попытка загрузить микропрограмму объёмом больше резервной области программ
[1]  0x12 - Нарушена последовательность выполнения команд при модификации памяти программ
[2]  0x13 - Приняты не все блоки микропрограммы
[3]  0x14 - Общее количество принятых блоков данных для модификации программ 
						не соответствует заявленному в предыдущем пакете
[4]  0x15 - Нарушена последовательность передачи блоков (в сторону увеличения)
[5]  0x16 - Ошибка CRC16 в постоянной памяти программ
[6]  0x31 - Нет связи с подчиненным контроллером
[7]  0x41 - Отсутствие EOL
[8]  0x43 - Длина пакета пакета первышает ёмкость буфера
[9]  0x44 - Задана нечетная длина BODY
[10] 0x45 - Ошибка CRC16
[11] 0x46 - Заданный код команды неподдерживается
[12] 0x48 - Неправильный пароль
[13] 0x49 - Запись в память запрещена
[14] 0x4B - Попытка чтения отсутствующей таблицы калибровок или идентификатора
[15] 0x4D - Длина BODY не соответствует LEN
[16] 0x52 - Данные BODY или его длина не корректны
[17] 0x61 - Переполнение буфера BSI при циклическом сборе данных
------------------ОШИБКИ-----------------------------*/