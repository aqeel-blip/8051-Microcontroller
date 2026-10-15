#include <reg52.h> 
#include "I2C.h"
#include "delay.h"
#include "LCD8bit.h"

unsigned int i;

void EepromWriteByte(unsigned char Data, unsigned char Address, unsigned char Page)
{
    I2CStart(); 
    I2CSend(0xA0 | (Page << 1));    
    I2CSend(Address); 
    I2CSend(Data);     
    I2CStop();             
    Delay_ms(5);        
}

unsigned char EepromReadByte(unsigned char Address, unsigned char Page)
{
    unsigned char Data;

    I2CStart();             
    I2CSend(0xA0 | (Page << 1));    
    I2CSend(Address); 
    I2CStart();        
    I2CSend(0xA1 | (Page << 1));            
    Data = I2CRead(); 
    I2CNak();
    I2CStop();        
    Delay_us(10);
    return Data;         
}

void EepromWriteNBytes(unsigned char Location, unsigned char *Pointer, char N, unsigned char Page)
{
    while (N != 0)
    {
        // Fixed: Swapped Data (*Pointer) and Address (Location) to match function signature
        EepromWriteByte(*Pointer, Location, Page);
        Location++;
        Pointer++;
        N--;        
    }
}

void EepromReadNBytes(unsigned char Location, unsigned char *Pointer, char N, unsigned char Page)
{
    while (N != 0)
    {
        *Pointer = EepromReadByte(Location, Page);
        Location++;
        Pointer++;
        N--;
    }
}

void EepromErasePage(unsigned char Page)
{
    unsigned int j;
    for (j = 0; j < 256; j++)
    {
        EepromWriteByte(0x00, (unsigned char)j, Page);    
    }
}

void EepromEraseAll()
{
    unsigned int j;
    for (j = 0; j < 256; j++)
    {
        EepromWriteByte(0x00, (unsigned char)j, 0);
        EepromWriteByte(0x00, (unsigned char)j, 1);
    }    
}

void main()
{
    // Updated string to "Aqeel Ahmad" formatted to 16 characters for LCD line length
    char Name[] = "  Aqeel Ahmad   ";
    unsigned char length = sizeof(Name) - 1; // 16 bytes

    initializeLCD();
    setCursor(0, 0);
    
    Disp("EEPROM 24C04");
    
    // Write "Aqeel Ahmad" to EEPROM
    for (i = 0; i < length; i++)
    {
        EepromWriteByte(Name[i], i, 0);
    }
    
    // Read back and display on line 2
    setCursor(0, 1);    
    for (i = 0; i < length; i++)
    {                
        Data(EepromReadByte(i, 0));
        Delay_ms(200);
    }

    while (1);
}
