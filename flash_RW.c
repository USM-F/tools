#include <p18f252.h>

//стирание блока
void ERASE_FLASHROM(far rom void * FLASH_Adress)
{
    TBLPTR = (unsigned short long) FLASH_Adress;

    EECON1bits.WREN  = 1;   // enable write to memory
    EECON1bits.FREE  = 1;   // Enable Row Erase operation

    TABLAT = INTCON;        // save interrupts  
    INTCON = 0;             // disable interrupts
    
    EECON2 = 0x55;      
    EECON2 = 0xAA;
    EECON1bits.WR = 1;      // start erase (CPU stall)

    INTCON = TABLAT;        // restore interrupts

    EECON1bits.WREN  = 0;   // disable write to memory
}

//запись блоками по 64
unsigned char   PROGRAM_BLOCK_FLASHROM(far ram void * RAM_Adress, far rom void * FLASH_Adress, unsigned char BlockCount)
{
 unsigned char  count1;
 unsigned char  count2;
 unsigned char* point;

    TBLPTR   = (unsigned short long) FLASH_Adress;
    TBLPTRL &= 0b11000000;

   _asm TBLRDPOSTDEC _endasm

    point = RAM_Adress;

    for(count1 = 0; count1 < BlockCount; count1++)
     {
        for(count2 = 0; count2 < 64; count2++)
         {
            TABLAT = *point++;
           _asm TBLWTPREINC _endasm
         };

//        EECON1bits.WPROG = 0;   // Program 64 bytes on the next WR command
        EECON1bits.FREE  = 0;   // Perform write only
        EECON1bits.WREN  = 1;   // enable write to memory

        TABLAT = INTCON;        // save interrupts
        INTCON = 0;             // disable interrupts
    
        EECON2 = 0x55;      
        EECON2 = 0xAA;
        EECON1bits.WR = 1;      // start program (CPU stall)

        INTCON = TABLAT;        // restore interrupts

        EECON1bits.WREN  = 0;   // disable write to memory
     };

    // Verify

    TBLPTR   = (unsigned short long) FLASH_Adress;
    TBLPTRL &= 0b11000000;

    for(count1 = 0; count1 < BlockCount; count1++)
     {
        for(count2 = 0; count2 < 64; count2++)
         {
           _asm TBLRDPOSTINC _endasm
            if(TABLAT != *(unsigned char*) RAM_Adress) return 0;
            RAM_Adress += 1;
         };
     };

    return 1;
}

//запись слова
unsigned char   PROGRAM_WORD_FLASHROM(unsigned int Word, far rom void * FLASH_Adress)
{
    TBLPTR   = (unsigned short long) FLASH_Adress;
    TBLPTRL &= 0b11111110;

   _asm TBLRDPOSTDEC _endasm

    TABLAT = (unsigned char)  Word;
   _asm TBLWTPREINC _endasm
    TABLAT = (unsigned char) (Word >> 8);
   _asm TBLWTPREINC _endasm

//    EECON1bits.WPROG = 1;   // Program 2 bytes on the next WR command
    EECON1bits.FREE  = 0;   // Perform write only
    EECON1bits.WREN  = 1;   // enable write to memory

    TABLAT = INTCON;        // save interrupts
    INTCON = 0;             // disable interrupts
    
    EECON2 = 0x55;      
    EECON2 = 0xAA;
    EECON1bits.WR = 1;      // start program (CPU stall)

    INTCON = TABLAT;        // restore interrupts

    EECON1bits.WREN  = 0;   // disable write to memory

    // Verify

    if(*((unsigned int *)FLASH_Adress) == Word) return 1;
                                          else  return 0;
}