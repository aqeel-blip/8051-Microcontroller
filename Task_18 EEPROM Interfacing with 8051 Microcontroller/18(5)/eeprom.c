#include "eeprom.h"

void EEPROM_WriteByte(uint16_t address, uint8_t dat) {
    // Low-level byte write logic (I2C AT24C02 or internal EEPROM SFRs)
    (void)address;
    (void)dat;
}

uint8_t EEPROM_ReadByte(uint16_t address) {
    // Low-level byte read logic (I2C AT24C02 or internal EEPROM SFRs)
    (void)address;
    return 0x00;
}

// Split 32-bit counter into 4 bytes using shift (>>) and bitwise AND (&)
void EEPROM_WriteCounter32(uint16_t start_address, uint32_t value) {
    uint8_t b0 = (uint8_t)(value & 0xFF);
    uint8_t b1 = (uint8_t)((value >> 8) & 0xFF);
    uint8_t b2 = (uint8_t)((value >> 16) & 0xFF);
    uint8_t b3 = (uint8_t)((value >> 24) & 0xFF);

    EEPROM_WriteByte(start_address,     b0);
    EEPROM_WriteByte(start_address + 1, b1);
    EEPROM_WriteByte(start_address + 2, b2);
    EEPROM_WriteByte(start_address + 3, b3);
}

// Reassemble 4 bytes into a 32-bit counter using shift (<<) and bitwise OR (|)
uint32_t EEPROM_ReadCounter32(uint16_t start_address) {
    uint8_t b0 = EEPROM_ReadByte(start_address);
    uint8_t b1 = EEPROM_ReadByte(start_address + 1);
    uint8_t b2 = EEPROM_ReadByte(start_address + 2);
    uint8_t b3 = EEPROM_ReadByte(start_address + 3);

    uint32_t value = ((uint32_t)b3 << 24) |
                     ((uint32_t)b2 << 16) |
                     ((uint32_t)b1 << 8)  |
                     ((uint32_t)b0);

    return value;
}