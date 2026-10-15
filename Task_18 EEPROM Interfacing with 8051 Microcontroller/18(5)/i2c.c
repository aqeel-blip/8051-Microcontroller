#include "i2c.h"

static void I2C_Delay(void) {
    unsigned char i = 2;
    while (i--);
}

void I2C_Start(void) {
    I2C_SDA = 1;
    I2C_SCL = 1;
    I2C_Delay();
    I2C_SDA = 0;
    I2C_Delay();
    I2C_SCL = 0;
}

void I2C_Stop(void) {
    I2C_SDA = 0;
    I2C_SCL = 1;
    I2C_Delay();
    I2C_SDA = 1;
    I2C_Delay();
}

void I2C_SendByte(unsigned char dat) {
    unsigned char i;
    for (i = 0; i < 8; i++) {
        I2C_SDA = (dat & 0x80) ? 1 : 0;
        dat <<= 1;
        I2C_SCL = 1;
        I2C_Delay();
        I2C_SCL = 0;
        I2C_Delay();
    }
    // Clock cycle for ACK bit
    I2C_SDA = 1;
    I2C_SCL = 1;
    I2C_Delay();
    I2C_SCL = 0;
}

unsigned char I2C_ReadByte(unsigned char ack) {
    unsigned char i, dat = 0;
    I2C_SDA = 1;
    for (i = 0; i < 8; i++) {
        dat <<= 1;
        I2C_SCL = 1;
        I2C_Delay();
        if (I2C_SDA) dat |= 0x01;
        I2C_SCL = 0;
        I2C_Delay();
    }
    // Send ACK / NACK
    I2C_SDA = ack ? 0 : 1;
    I2C_SCL = 1;
    I2C_Delay();
    I2C_SCL = 0;
    I2C_SDA = 1;
    
    return dat;
}

void EEPROM_WriteByte(unsigned char addr, unsigned char dat) {
    I2C_Start();
    I2C_SendByte(0xA0); // AT24C02 Write Device Address
    I2C_SendByte(addr); // Target Memory Address
    I2C_SendByte(dat);  // Byte to Write
    I2C_Stop();
    
    // Internal write cycle delay (~5ms required for AT24C02)
    {
        unsigned int i, j;
        for (i = 5; i > 0; i--)
            for (j = 110; j > 0; j--);
    }
}

unsigned char EEPROM_ReadByte(unsigned char addr) {
    unsigned char dat;
    I2C_Start();
    I2C_SendByte(0xA0); // Dummy Write command
    I2C_SendByte(addr); // Memory Address to read
    I2C_Start();
    I2C_SendByte(0xA1); // Read Command
    dat = I2C_ReadByte(0); // NACK to terminate read
    I2C_Stop();
    return dat;
}

// Write 4-byte (32-bit) counter to 4 adjacent locations using Shift and & operators
void EEPROM_WriteCounter32(unsigned char startAddr, unsigned long counter) {
    unsigned char b0 = (unsigned char)(counter & 0xFF);
    unsigned char b1 = (unsigned char)((counter >> 8) & 0xFF);
    unsigned char b2 = (unsigned char)((counter >> 16) & 0xFF);
    unsigned char b3 = (unsigned char)((counter >> 24) & 0xFF);

    EEPROM_WriteByte(startAddr + 0, b0);
    EEPROM_WriteByte(startAddr + 1, b1);
    EEPROM_WriteByte(startAddr + 2, b2);
    EEPROM_WriteByte(startAddr + 3, b3);
}

// Read 4 adjacent locations and assemble into 32-bit value using Shift and | operators
unsigned long EEPROM_ReadCounter32(unsigned char startAddr) {
    unsigned long b0 = EEPROM_ReadByte(startAddr + 0);
    unsigned long b1 = EEPROM_ReadByte(startAddr + 1);
    unsigned long b2 = EEPROM_ReadByte(startAddr + 2);
    unsigned long b3 = EEPROM_ReadByte(startAddr + 3);

    return (b0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
}