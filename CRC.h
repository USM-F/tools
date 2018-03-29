#ifndef __CRC_H
#define __CRC_H

#define POLY	0x1021

extern unsigned int  	CRC_16;

struct bits
{
unsigned short b0 : 1;
unsigned short b1 : 1;
unsigned short b2 : 1;
unsigned short b3 : 1;
unsigned short b4 : 1;
unsigned short b5 : 1;
unsigned short b6 : 1;
unsigned short b7 : 1;
} ;


unsigned short getCrc16(unsigned char *dataBlock, unsigned int len);


#endif



