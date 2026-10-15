#include "LCD.h"

void Delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
}

void LCD_Command(unsigned char cmd) {
    LCD_RS = 0;
    LCD_WR = 0;
    LCD_DATA = cmd;
    LCD_E = 1;
    Delay_ms(2);
    LCD_E = 0;
}

void LCD_Char(unsigned char dat) {
    LCD_RS = 1;
    LCD_WR = 0;
    LCD_DATA = dat;
    LCD_E = 1;
    Delay_ms(2);
    LCD_E = 0;
}

void LCD_Init(void) {
    LCD_Command(0x38); // 8-bit mode, 2 lines, 5x7 font
    LCD_Command(0x0C); // Display ON, Cursor OFF
    LCD_Command(0x06); // Entry mode: increment automatically
    LCD_Command(0x01); // Clear display
    Delay_ms(5);
}

void LCD_String(char *str) {
    while (*str) {
        LCD_Char(*str++);
    }
}

void LCD_SetCursor(unsigned char row, unsigned char col) {
    if (row == 0)      LCD_Command(0x80 + col);
    else if (row == 1) LCD_Command(0xC0 + col);
}

void LCD_PrintTwoDigits(unsigned char val) {
    LCD_Char((val / 10) + '0');
    LCD_Char((val % 10) + '0');
}