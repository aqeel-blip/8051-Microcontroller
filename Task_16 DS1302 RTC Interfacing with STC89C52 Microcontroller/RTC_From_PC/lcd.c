#include "lcd.h"

static void LCD_Delay(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
}

void LCD_WriteCmd(unsigned char cmd) {
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_DATA_PORT = cmd;
    LCD_EN = 1;
    LCD_Delay(1);
    LCD_EN = 0;
    LCD_Delay(1);
}

void LCD_WriteData(unsigned char dat) {
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_DATA_PORT = dat;
    LCD_EN = 1;
    LCD_Delay(1);
    LCD_EN = 0;
    LCD_Delay(1);
}

void LCD_Init(void) {
    LCD_Delay(15);
    LCD_WriteCmd(0x38); // 8-bit bus, 2 lines, 5x7 font
    LCD_WriteCmd(0x0C); // Display ON, Cursor OFF
    LCD_WriteCmd(0x06); // Auto-increment cursor
    LCD_WriteCmd(0x01); // Clear display
    LCD_Delay(2);
}

void LCD_Clear(void) {
    LCD_WriteCmd(0x01);
    LCD_Delay(2);
}

void LCD_SetCursor(unsigned char row, unsigned char col) {
    if (row == 1) {
        LCD_WriteCmd(0x80 + (col - 1));
    } else if (row == 2) {
        LCD_WriteCmd(0xC0 + (col - 1));
    }
}

void LCD_ShowString(unsigned char row, unsigned char col, char *str) {
    LCD_SetCursor(row, col);
    while (*str) {
        LCD_WriteData(*str++);
    }
}