#include <reg52.h>
#include <stdio.h>
#include "lcd.h"

// DS1302 Pin Mapping (PRECHIN A2 Board)
sbit DS1302_IO   = P3^4;
sbit DS1302_CE   = P3^5;
sbit DS1302_SCLK = P3^6;

// DS1302 Register Addresses
#define REG_SEC   0x80
#define REG_MIN   0x82
#define REG_HOUR  0x84
#define REG_DATE  0x86
#define REG_MONTH 0x88
#define REG_YEAR  0x8C
#define REG_WP    0x8E

char rxdata[16];
volatile unsigned char rxindex = 0;
volatile bit string_received = 0;

unsigned char BCD_To_DEC(unsigned char bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

unsigned char DEC_To_BCD(unsigned char dec) {
    return (((dec / 10) << 4) | (dec % 10));
}

void UART_Init(void) {
    SCON = 0x50;  // 8-bit UART
    TMOD &= 0x0F;
    TMOD |= 0x20; // Timer 1 Mode 2
    TH1 = 0xFD;   // 9600 Baud Rate @ 11.0592 MHz
    TL1 = 0xFD;
    TR1 = 1;
    ES  = 1;
    EA  = 1;
}

void UART_SendChar(char ch) {
    SBUF = ch;
    while (!TI);
    TI = 0;
}

void UART_SendString(char *str) {
    while (*str) {
        UART_SendChar(*str++);
    }
}

void DS1302_WriteByte(unsigned char dat) {
    unsigned char i;
    for (i = 0; i < 8; i++) {
        DS1302_IO = dat & 0x01;
        DS1302_SCLK = 1;
        DS1302_SCLK = 0;
        dat >>= 1;
    }
}

unsigned char DS1302_ReadByte(void) {
    unsigned char i, dat = 0;
    DS1302_IO = 1; // Release I/O latch for input reading
    for (i = 0; i < 8; i++) {
        dat >>= 1;
        if (DS1302_IO) dat |= 0x80;
        DS1302_SCLK = 1;
        DS1302_SCLK = 0;
    }
    return dat;
}

void DS1302_WriteReg(unsigned char reg, unsigned char dat) {
    DS1302_CE = 0;
    DS1302_SCLK = 0;
    DS1302_CE = 1;
    DS1302_WriteByte(reg);
    DS1302_WriteByte(dat);
    DS1302_CE = 0;
}

unsigned char DS1302_ReadReg(unsigned char reg) {
    unsigned char dat;
    DS1302_CE = 0;
    DS1302_SCLK = 0;
    DS1302_CE = 1;
    DS1302_WriteByte(reg | 0x01);
    dat = DS1302_ReadByte();
    DS1302_CE = 0;
    return dat;
}

void DS1302_Init(void) {
    unsigned char sec;
    DS1302_WriteReg(REG_WP, 0x00); // Disable Write Protect
    
    sec = DS1302_ReadReg(REG_SEC);
    if (sec & 0x80) {              // Clear Clock Halt (CH) bit if set
        DS1302_WriteReg(REG_SEC, 0x00);
    }
}

void DS1302_SetTime(unsigned char y, unsigned char m, unsigned char d, unsigned char h, unsigned char min, unsigned char s) {
    DS1302_WriteReg(REG_WP, 0x00);
    DS1302_WriteReg(REG_YEAR, DEC_To_BCD(y));
    DS1302_WriteReg(REG_MONTH, DEC_To_BCD(m));
    DS1302_WriteReg(REG_DATE, DEC_To_BCD(d));
    DS1302_WriteReg(REG_HOUR, DEC_To_BCD(h));
    DS1302_WriteReg(REG_MIN, DEC_To_BCD(min));
    DS1302_WriteReg(REG_SEC, DEC_To_BCD(s) & 0x7F);
    DS1302_WriteReg(REG_WP, 0x80);
}

void Delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
}

void main(void) {
    char dateBuf[17];
    char timeBuf[17];
    unsigned char y, m, d, h, min, s;

    UART_Init();
    LCD_Init();
    DS1302_Init();

    LCD_ShowString(1, 1, " RTC Initialized ");
    Delay_ms(1000);
    LCD_Clear();

    while (1) {
        if (string_received) {
            if (rxindex >= 12) {
                y   = (rxdata[0] - '0') * 10 + (rxdata[1] - '0');
                m   = (rxdata[2] - '0') * 10 + (rxdata[3] - '0');
                d   = (rxdata[4] - '0') * 10 + (rxdata[5] - '0');
                h   = (rxdata[6] - '0') * 10 + (rxdata[7] - '0');
                min = (rxdata[8] - '0') * 10 + (rxdata[9] - '0');
                s   = (rxdata[10] - '0') * 10 + (rxdata[11] - '0');

                DS1302_SetTime(y, m, d, h, min, s);
                UART_SendString("RTC Updated!\r\n");
            }
            rxindex = 0;
            string_received = 0;
        }

        // Read RTC registers
        y   = BCD_To_DEC(DS1302_ReadReg(REG_YEAR));
        m   = BCD_To_DEC(DS1302_ReadReg(REG_MONTH));
        d   = BCD_To_DEC(DS1302_ReadReg(REG_DATE));
        h   = BCD_To_DEC(DS1302_ReadReg(REG_HOUR));
        min = BCD_To_DEC(DS1302_ReadReg(REG_MIN));
        s   = BCD_To_DEC(DS1302_ReadReg(REG_SEC) & 0x7F);

        // Format for LCD using Keil unsigned byte specifier %02bu
        sprintf(dateBuf, "Date: 20%02bu-%02bu-%02bu", y, m, d);
        sprintf(timeBuf, "Time: %02bu:%02bu:%02bu", h, min, s);

        LCD_ShowString(1, 1, dateBuf);
        LCD_ShowString(2, 1, timeBuf);

        Delay_ms(500);
    }
}

void UART_ISR(void) interrupt 4 {
    if (RI) {
        char ch = SBUF;
        RI = 0;
        if (ch != '\r' && ch != '\n') {
            if (rxindex < 15) {
                rxdata[rxindex++] = ch;
            }
        } else {
            rxdata[rxindex] = '\0';
            string_received = 1;
        }
    }
}