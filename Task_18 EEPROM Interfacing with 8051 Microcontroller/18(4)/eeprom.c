#include "eeprom.h"
#include <intrins.h>

static void I2C_Delay(void) {
    _nop_();
    _nop_();
    _nop_();
    _nop_();
}

void I2C_Start(void) {
    SDA = 1;
    SCL = 1;
    I2C_Delay();
    SDA = 0;
    I2C_Delay();
    SCL = 0;
}

void I2C_Stop(void) {
    SDA = 0;
    SCL = 1;
    I2C_Delay();
    SDA = 1;
    I2C_Delay();
}

unsigned char I2C_SendByte(unsigned char dat) {
    unsigned char i, ack;
    for (i = 0; i < 8; i++) {
        SDA = (dat & 0x80) ? 1 : 0;
        dat <<= 1;
        SCL = 1;
        I2C_Delay();
        SCL = 0;
        I2C_Delay();
    }
    SDA = 1; // Release line for ACK
    SCL = 1;
    I2C_Delay();
    ack = SDA;
    SCL = 0;
    return ack;
}

unsigned char I2C_ReadByte(unsigned char ack) {
    unsigned char i, dat = 0;
    SDA = 1;
    for (i = 0; i < 8; i++) {
        dat <<= 1;
        SCL = 1;
        I2C_Delay();
        if (SDA) dat |= 0x01;
        SCL = 0;
        I2C_Delay();
    }
    SDA = ack ? 0 : 1;
    SCL = 1;
    I2C_Delay();
    SCL = 0;
    SDA = 1;
    return dat;
}

void EEPROM_WriteByte(unsigned char addr, unsigned char dat) {
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR);       // Device Write Address
    I2C_SendByte(addr);              // Memory Word Address
    I2C_SendByte(dat);               // Data byte
    I2C_Stop();
    
    // Self-timed write cycle delay (max 5ms for 24C02)
    {
        unsigned int delay = 1000;
        while(delay--);
    }
}

unsigned char EEPROM_ReadByte(unsigned char addr) {
    unsigned char dat;
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR);       // Dummy Write
    I2C_SendByte(addr);              // Word Address
    
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR | 0x01);// Read command
    dat = I2C_ReadByte(0);           // Read single byte with NACK
    I2C_Stop();
    
    return dat;
}