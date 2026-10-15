#ifndef EEPROM_H
#define EEPROM_H

#include <reg51.h>

#ifndef TYPES_DEFINED
#define TYPES_DEFINED
typedef unsigned char uint8_t;
typedef unsigned int  uint16_t;
typedef unsigned long uint32_t;
#endif

// EEPROM Byte Read/Write Prototypes
void EEPROM_WriteByte(uint16_t address, uint8_t dat);
uint8_t EEPROM_ReadByte(uint16_t address);

// 32-Bit (4-Byte) EEPROM Functions
void EEPROM_WriteCounter32(uint16_t start_address, uint32_t value);
uint32_t EEPROM_ReadCounter32(uint16_t start_address);

#endif