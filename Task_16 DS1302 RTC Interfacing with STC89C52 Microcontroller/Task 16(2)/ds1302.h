#ifndef __DS1302_H__
#define __DS1302_H__

#include <reg52.h>

// Pin Configuration from Schematic
sbit DS1302_SCLK = P3^6;
sbit DS1302_IO   = P3^4;
sbit DS1302_CE   = P3^5;

// DS1302 Register Addresses (Write / Read)
#define DS1302_SEC_WRITE   0x80
#define DS1302_SEC_READ    0x81
#define DS1302_MIN_WRITE   0x82
#define DS1302_MIN_READ    0x83
#define DS1302_HOUR_WRITE  0x84
#define DS1302_HOUR_READ   0x85
#define DS1302_DATE_WRITE  0x86
#define DS1302_DATE_READ   0x87
#define DS1302_MONTH_WRITE 0x88
#define DS1302_MONTH_READ  0x89
#define DS1302_DAY_WRITE   0x8A
#define DS1302_DAY_READ    0x8B
#define DS1302_YEAR_WRITE  0x8C
#define DS1302_YEAR_READ   0x8D
#define DS1302_WP_WRITE    0x8E

// Time structure container
typedef struct {
    unsigned char year;   // 00 - 99
    unsigned char month;  // 01 - 12
    unsigned char date;   // 01 - 31
    unsigned char day;    // 01 - 07 (1 = Mon, 7 = Sun)
    unsigned char hour;   // 00 - 23
    unsigned char min;    // 00 - 59
    unsigned char sec;    // 00 - 59
} DS1302_Time;

// Function Prototypes
void DS1302_Init(void);
void DS1302_WriteReg(unsigned char cmd, unsigned char dat);
unsigned char DS1302_ReadReg(unsigned char cmd);
void DS1302_SetTime(DS1302_Time *time);
void DS1302_GetTime(DS1302_Time *time);

#endif