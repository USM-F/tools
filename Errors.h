#ifndef __ERRORS_H
#define __ERRORS_H

//Файл кодов ошибок

//MEMORY
#define NO_ERRORS									0x00			//Ошибок нет
#define BIG_DATA_WRITE_ERROR						0x11			//Попытка записать слишком большой объём данных в постоянную память
#define SEQUENCE_OF_CMD_DATA_WRITE_ERROR			0x12			//Нарушена последовательность выполнения команд при записи данных в постоянную память
#define NOT_ALL_BLOCKS_DATA_RECEIVED_ERROR			0x13			//Приняты не все блоки данных при записи в постоянную память
#define WRONG_BLOCKS_DATA_WRITE_ERROR				0x14			//Общее количество блоков данных при записи в постоянную память не соответствует заявленному в предыдущем пакете
#define WRONG_BLOCKS_DATA_TRANSFER_ERROR			0x15			//Нарушена последовательность передачи блоков (в сторону увеличения) при записи в постоянную память
#define CRC16_DATA_SAVED_ERROR						0x16			//Ошибка CRC16 сохранённых данных в постоянной памяти
#define WRONG_TRANSFER_LENGHT_DATA_ERROR			0x17			//Длина передаваемых данных в пакете при записи в постоянную память не соответствует требуемой длине блока
//FDISK
#define TRANSFER_FDISK_DATA_ERROR					0x23			//Передаваемый пакет данных из FDISK содержит ошибку хотя бы в одном из секторов и её невозможно исправить
#define BAD_BLOCK_FDISK_ERROR						0x24			//FDISK не пригоден к использованию. Много плохих кластеров
#define FORMAT_FDISK_ERROR							0x25			//Требуется форматирование FDISK
#define SECTOR_NOT_EXIST_FDISK_ERROR				0x27			//Запрашиваемый сектор в FDISK не существует
#define FULL_FDISK_ERROR							0x28			//Запрашиваемый сектор в FDISK не существует
//CONTROLLER
#define ANSWER_SLAVE_C0NTROLLER_ERROR				0x31			//Нет ответа от подчиненного контроллера
#define ANSWER_CDA_ERROR							0x32			//Нет ответа от CDA
#define ANSWER_MEASURE_CHANNEL_ERROR				0x33			//Нет ответа от измерительного канала
//PROTOCOL
#define EOF_ERROR									0x41			//Нерегламентированный код EOF или его отсутствие, приоритет 4
#define OVER_BUF_LENGHT_ERROR						0x43			//Длина принимаемого пакета превышает ёмкость буфера обмена, приоритет 1
#define ODD_BODY_ERROR								0x44			//Задана нечетная длина BODY, приоритет 2
#define CRC16_RECEIVED_ERROR						0x45			//Ошибка CRC16 в принятом пакете, приоритет 5
#define COMMAND_NOT_SUPPORT_ERROR					0x46			//Заданный код команды не поддерживается, приоритет 6
#define ALLOW_EXCUTE_COMMAND_ERROR					0x47			//Нет разрешения на выполнение данной команды
#define WRONG_PASSWORD_ERROR						0x48			//Неправильный пароль
#define WRITE_DATA_ERROR							0x49			//Запись в постоянную память запрещена
#define REQUEST_DATA_ERROR							0x4B			//Запрашиваемых данных нет
#define BODY_NOT_EQUALS_LENGHT_ERROR				0x4D			//Длина BODY не соответствует LEN, приоритет 3
#define COMMAND_FDISK_ERROR							0x4E			//Нельзя исполнить команду в режиме "РЕГИСТРАЦИЯ" или тестирования FDISK
//TECHNOLOGICAL
#define WELL_CLT_INFORMATION_ERROR					0x50			//Отсутствие данных с информацией по скважине в постоянной памяти CLT
#define POLL_CLT_CDA_PARAMETERS_ERROR				0x51			//Отсутствие параметров опроса для CLT или CDA, приоритет 7
#define BODY_DATA_PARAMETERS_NOT_REGULATED_ERROR	0x52			//Данные в BODY командного пакета не регламентированы
#define SET_TIME_ERROR								0x53			//Напряжение питания ниже допустимого значения
#define LOW_VOLTAGE_ERROR							0x54			//Не установлены время, дата или будильник
#define POLL_TOOL_LESS_TIME_ERROR					0x56			//Время начала регистрации хотя бы для одного устройства меньше текущего времени
#define CYCLE_CDA_TIME_ERROR						0x57			//Время цикла опроса хотя бы одного CDA меньше затребованного времени занятости магистрали другими CDA
#define CDA_NOT_111_ERROR							0x58			//В старшем байте адреса CDA три младших бита не равны 111
#define REGISTRATION_NOT_PARAMETERS_ERROR			0x59			//Не заданы технологические параметры регистрации
//USERS
#define INCOMPLETE_DATA_ERROR						0xC0			//Неполные данные, если один из подчиненных БСИ не ответил

#endif
/*
CDA 		- система сбора данных от измерительных зондов.
CLT			- головной контроллер (система сбора данных от CDA).
ADDR		- двухбайтный адрес логического устройства.
CMD			- двухбайтный код команды.
BODY		- данные в теле пакета.
LEN			- длина тела пакета в байтах.
CRC16		- контрольная сумма по стандарту CCITT CRC-16, полином 0x1021 (код инициализации 0xFFFF).
ERR			- байт кода ошибки.
EOF 		- двухбайтный признак конца пакета, код 0x00FF.
FDISK		- съёмная флэш память.
ПРИБОР		- устройство, предназначенное для проведение ГИС, заключённое в охранный кожух и имеющее верхнюю и нижнюю головки.
МОДУЛЬ		- заменяемое измерительное или управляющее устройство, размещённое внутри прибора.
*/