#ifndef __LCD1602_H__
#define __LCD1602_H__

#include <reg52.h>

// Pin Definitions based on PRECHIN A2 Schematic
sbit LCD_RS = P2^6;
sbit LCD_RW = P2^5;
sbit LCD_EN = P2^7;

#define LCD_DATA_PORT P0

// Function Prototypes
void LCD_Init(void);
void LCD_WriteCommand(unsigned char cmd);
void LCD_WriteData(unsigned char dat);
void LCD_SetCursor(unsigned char row, unsigned char col);
void LCD_ShowString(unsigned char row, unsigned char col, char *str);
void LCD_ShowNum(unsigned char row, unsigned char col, unsigned long num, unsigned char length);

#endif