#ifndef __I2C_H__
#define __I2C_H__

#include <reg52.h>

// I2C Pin Definitions for AT24C02
sbit I2C_SDA = P2^0;
sbit I2C_SCL = P2^1;

// Function Prototypes
void I2C_Start(void);
void I2C_Stop(void);
void I2C_SendByte(unsigned char dat);
unsigned char I2C_ReadByte(unsigned char ack);

void EEPROM_WriteByte(unsigned char addr, unsigned char dat);
unsigned char EEPROM_ReadByte(unsigned char addr);

void EEPROM_WriteCounter32(unsigned char startAddr, unsigned long counter);
unsigned long EEPROM_ReadCounter32(unsigned char startAddr);

#endif