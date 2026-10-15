#ifndef __LCD_H__
#define __LCD_H__

#include <reg52.h>

// Control pin definitions for A2 Development Board
sbit LCD_RS = P2^6;
sbit LCD_WR = P2^5;
sbit LCD_E  = P2^7;

#define LCD_DATA P0

// Function prototypes
void Delay_ms(unsigned int ms);
void LCD_Command(unsigned char cmd);
void LCD_Char(unsigned char dat);
void LCD_Init(void);
void LCD_String(char *str);
void LCD_SetCursor(unsigned char row, unsigned char col);
void LCD_PrintTwoDigits(unsigned char val);

#endif