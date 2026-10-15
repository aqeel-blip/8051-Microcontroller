#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <reg52.h>

// I2C Pin Configuration (A2 Development Board schematic)
sbit SDA = P2^0;
sbit SCL = P2^1;

// Device Address for 24C02
#define EEPROM_ADDR 0xA0

// Minimum write interval to ensure 10-year lifespan (315.36s rounded up)
#define EEPROM_WRITE_INTERVAL_SEC 316 

// Function Declarations
void I2C_Start(void);
void I2C_Stop(void);
unsigned char I2C_SendByte(unsigned char dat);
unsigned char I2C_ReadByte(unsigned char ack);

void EEPROM_WriteByte(unsigned char addr, unsigned char dat);
unsigned char EEPROM_ReadByte(unsigned char addr);

#endif