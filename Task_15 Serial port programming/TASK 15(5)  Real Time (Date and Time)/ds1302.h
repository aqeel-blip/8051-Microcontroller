#ifndef __DS1302_H__
#define __DS1302_H__

#include <reg52.h> // Header for standard 8051 microcontrollers (Keil C51)

// ==========================================
// Hardware Pin Definitions
// Adjust port pins according to your wiring
// ==========================================
sbit DS1302_IO   = P3^5;
sbit DS1302_SCLK = P3^6;
sbit DS1302_CE   = P3^7;  // RST / Chip Enable pin

// ==========================================
// Global Time/Date Variables (Shared)
// ==========================================
extern unsigned char hour, minute, second;
extern unsigned char day, month, year;

// ==========================================
// Public Function Prototypes
// ==========================================

/**
 * @brief Sets the DS1302 date and time in 24-hour mode.
 * @param h   Hour (0-23)
 * @param m   Minute (0-59)
 * @param s   Second (0-59)
 * @param d   Day of month (1-31)
 * @param mon Month (1-12)
 * @param y   Year (00-99)
 */
void DS1302_Set_Time(unsigned char h, unsigned char m, unsigned char s,
                     unsigned char d, unsigned char mon, unsigned char y);

/**
 * @brief Reads current time/date from DS1302 into global variables.
 */
void DS1302_Read_Time(void);

#endif /* __DS1302_H__ */