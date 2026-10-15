#include "lcd.h"
#include "lcd1602.h"

static void LCD_DelayMs(unsigned int ms) {
    unsigned int i, j;
    for (i = ms; i > 0; i--)
        for (j = 110; j > 0; j--);
}

void LCD_WriteCommand(unsigned char cmd) {
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_DATA_PORT = cmd;
    LCD_EN = 1;
    LCD_DelayMs(1);
    LCD_EN = 0;
}

void LCD_WriteData(unsigned char dat) {
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_DATA_PORT = dat;
    LCD_EN = 1;
    LCD_DelayMs(1);
    LCD_EN = 0;
}

void LCD_Init(void) {
    LCD_DelayMs(15);
    LCD_WriteCommand(0x38); // 8-bit mode, 2-line display, 5x7 font
    LCD_WriteCommand(0x0C); // Display ON, Cursor OFF, Blink OFF
    LCD_WriteCommand(0x06); // Auto-increment cursor
    LCD_WriteCommand(0x01); // Clear display
    LCD_DelayMs(5);
}

void LCD_SetCursor(unsigned char row, unsigned char col) {
    if (row == 1) {
        LCD_WriteCommand(0x80 + (col - 1));
    } else if (row == 2) {
        LCD_WriteCommand(0xC0 + (col - 1));
    }
}

void LCD_ShowString(unsigned char row, unsigned char col, char *str) {
    LCD_SetCursor(row, col);
    while (*str) {
        LCD_WriteData(*str++);
    }
}

void LCD_ShowNum(unsigned char row, unsigned char col, unsigned long num, unsigned char length) {
    unsigned char i;
    char buf[11];
    
    for (i = 0; i < length; i++) {
        buf[length - 1 - i] = (num % 10) + '0';
        num /= 10;
    }
    buf[length] = '\0';
    
    LCD_ShowString(row, col, buf);
}