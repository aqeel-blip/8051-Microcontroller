#ifndef __LCD_H__
#define __LCD_H__

#include <reg52.h>

// PRECHIN A2 LCD1602 Pin Mapping
sbit LCD_RS = P2^6;
sbit LCD_RW = P2^5;
sbit LCD_EN = P2^7;

#define LCD_DATA_PORT P0

void LCD_Init(void);
void LCD_WriteCmd(unsigned char cmd);
void LCD_WriteData(unsigned char dat);
void LCD_SetCursor(unsigned char row, unsigned char col);
void LCD_ShowString(unsigned char row, unsigned char col, char *str);
void LCD_Clear(void);

#endif