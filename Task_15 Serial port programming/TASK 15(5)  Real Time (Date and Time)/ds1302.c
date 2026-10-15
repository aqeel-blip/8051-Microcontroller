#include "ds1302.h"

// Global time/date variables (read from RTC)
unsigned char hour, minute, second;
unsigned char day, month, year;

// ---- Small delay for DS1302 timing (about 5-10 µs) ----
static void ds1302_delay(void) {
    unsigned char i;
    for (i = 0; i < 20; i++);
}

// ---- Write one byte (LSB first) ----
static void DS1302_WriteByte(unsigned char dat) {
    unsigned char i;
    for (i = 0; i < 8; i++) {
        DS1302_IO = dat & 0x01;
        ds1302_delay();
        DS1302_SCLK = 1;
        ds1302_delay();
        DS1302_SCLK = 0;
        ds1302_delay();
        dat >>= 1;
    }
}

// ---- Read one byte (LSB first) ----
static unsigned char DS1302_ReadByte(void) {
    unsigned char i, dat = 0;
    DS1302_IO = 1;                // release bus
    for (i = 0; i < 8; i++) {
        dat >>= 1;
        if (DS1302_IO) dat |= 0x80;
        DS1302_SCLK = 1;
        ds1302_delay();
        DS1302_SCLK = 0;
        ds1302_delay();
    }
    return dat;
}

// ---- Write to a register ----
static void DS1302_Write(unsigned char addr, unsigned char dat) {
    DS1302_CE = 1;
    ds1302_delay();
    DS1302_WriteByte(addr);       // address with write bit (0)
    DS1302_WriteByte(dat);
    DS1302_CE = 0;
    ds1302_delay();
}

// ---- Read from a register ----
static unsigned char DS1302_Read(unsigned char addr) {
    unsigned char dat;
    DS1302_CE = 1;
    ds1302_delay();
    DS1302_WriteByte(addr | 0x01); // address with read bit (1)
    dat = DS1302_ReadByte();
    DS1302_CE = 0;
    ds1302_delay();
    return dat;
}

// ---- BCD <-> Binary conversions ----
static unsigned char BCD_to_Bin(unsigned char bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static unsigned char Bin_to_BCD(unsigned char bin) {
    return ((bin / 10) << 4) | (bin % 10);
}

// ---- Set the DS1302 time and date ----
void DS1302_Set_Time(unsigned char h, unsigned char m, unsigned char s,
                     unsigned char d, unsigned char mon, unsigned char y) {
    // Disable write protection
    DS1302_Write(0x8E, 0x00);
    
    // Write seconds (clear clock halt bit – bit7 = 0)
    DS1302_Write(0x80, Bin_to_BCD(s) & 0x7F);
    DS1302_Write(0x82, Bin_to_BCD(m));
    // Hours: bit6 = 0 => 24-hour mode, bit5 = 0 (no 12/24 flag)
    DS1302_Write(0x84, Bin_to_BCD(h) & 0x3F);
    DS1302_Write(0x86, Bin_to_BCD(d));
    DS1302_Write(0x88, Bin_to_BCD(mon));
    DS1302_Write(0x8C, Bin_to_BCD(y));
    
    // Re-enable write protection
    DS1302_Write(0x8E, 0x80);
}

// ---- Read current time and date into global variables ----
void DS1302_Read_Time(void) {
    second = BCD_to_Bin(DS1302_Read(0x81) & 0x7F);   // mask halt bit
    minute = BCD_to_Bin(DS1302_Read(0x83));
    hour   = BCD_to_Bin(DS1302_Read(0x85) & 0x3F);   // mask 12/24 flags
    day    = BCD_to_Bin(DS1302_Read(0x87));
    month  = BCD_to_Bin(DS1302_Read(0x89));
    year   = BCD_to_Bin(DS1302_Read(0x8D));
}