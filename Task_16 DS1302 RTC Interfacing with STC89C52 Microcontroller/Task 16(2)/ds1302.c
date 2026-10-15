#include "ds1302.h"

// Helper BCD Conversions
static unsigned char BCDToDec(unsigned char bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static unsigned char DecToBCD(unsigned char dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// Low-level bit-bang write (LSB first)
static void DS1302_WriteByte(unsigned char dat) {
    unsigned char i;
    for (i = 0; i < 8; i++) {
        DS1302_IO = dat & 0x01;
        DS1302_SCLK = 1;
        DS1302_SCLK = 0;
        dat >>= 1;
    }
}

// Low-level bit-bang read (LSB first)
static unsigned char DS1302_ReadByte(void) {
    unsigned char i, dat = 0;
    for (i = 0; i < 8; i++) {
        dat >>= 1;
        if (DS1302_IO) {
            dat |= 0x80;
        }
        DS1302_SCLK = 1;
        DS1302_SCLK = 0;
    }
    return dat;
}

// Write command/data to DS1302 register
void DS1302_WriteReg(unsigned char cmd, unsigned char dat) {
    DS1302_CE = 0;
    DS1302_SCLK = 0;
    DS1302_CE = 1;
    DS1302_WriteByte(cmd);
    DS1302_WriteByte(dat);
    DS1302_CE = 0;
}

// Read data from DS1302 register
unsigned char DS1302_ReadReg(unsigned char cmd) {
    unsigned char dat;
    DS1302_CE = 0;
    DS1302_SCLK = 0;
    DS1302_CE = 1;
    DS1302_WriteByte(cmd);
    dat = DS1302_ReadByte();
    DS1302_CE = 0;
    return dat;
}

// Initialize lines and clear Write Protect
void DS1302_Init(void) {
    DS1302_CE = 0;
    DS1302_SCLK = 0;
    DS1302_WriteReg(DS1302_WP_WRITE, 0x00); // Disable write protection
}

// Write complete time/date structure
void DS1302_SetTime(DS1302_Time *time) {
    DS1302_WriteReg(DS1302_WP_WRITE, 0x00); // Disable write protection
    
    DS1302_WriteReg(DS1302_SEC_WRITE, DecToBCD(time->sec));
    DS1302_WriteReg(DS1302_MIN_WRITE, DecToBCD(time->min));
    DS1302_WriteReg(DS1302_HOUR_WRITE, DecToBCD(time->hour));
    DS1302_WriteReg(DS1302_DATE_WRITE, DecToBCD(time->date));
    DS1302_WriteReg(DS1302_MONTH_WRITE, DecToBCD(time->month));
    DS1302_WriteReg(DS1302_DAY_WRITE, DecToBCD(time->day));
    DS1302_WriteReg(DS1302_YEAR_WRITE, DecToBCD(time->year));
    
    DS1302_WriteReg(DS1302_WP_WRITE, 0x80); // Enable write protection
}

// Read complete time/date structure
void DS1302_GetTime(DS1302_Time *time) {
    time->sec   = BCDToDec(DS1302_ReadReg(DS1302_SEC_READ) & 0x7F);
    time->min   = BCDToDec(DS1302_ReadReg(DS1302_MIN_READ) & 0x7F);
    time->hour  = BCDToDec(DS1302_ReadReg(DS1302_HOUR_READ) & 0x3F);
    time->date  = BCDToDec(DS1302_ReadReg(DS1302_DATE_READ) & 0x3F);
    time->month = BCDToDec(DS1302_ReadReg(DS1302_MONTH_READ) & 0x1F);
    time->day   = BCDToDec(DS1302_ReadReg(DS1302_DAY_READ) & 0x07);
    time->year  = BCDToDec(DS1302_ReadReg(DS1302_YEAR_READ));
}