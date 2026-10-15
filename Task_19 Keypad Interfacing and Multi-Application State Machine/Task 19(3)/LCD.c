#include <reg51.h>
#include "LCD.h"

sbit RS = P2^0;
sbit RW = P2^1;
sbit EN = P2^2;
#define LCD_PORT P0

static void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 120; j++);
}

void LCD_Command(unsigned char cmd) {
    LCD_PORT = cmd;
    RS = 0;
    RW = 0;
    EN = 1;
    delay_ms(2);
    EN = 0;
}

void LCD_Char(unsigned char char_data) {
    LCD_PORT = char_data;
    RS = 1;
    RW = 0;
    EN = 1;
    delay_ms(2);
    EN = 0;
}

void LCD_Init(void) {
    delay_ms(20);
    LCD_Command(0x38); // 8-bit mode, 2-line, 5x7 font
    LCD_Command(0x0C); // Display ON, Cursor OFF
    LCD_Command(0x06); // Entry mode set
    LCD_Command(0x01); // Clear display
    delay_ms(2);
}

void LCD_String(char *str) {
    while(*str) {
        LCD_Char(*str++);
    }
}

void LCD_Clear(void) {
    LCD_Command(0x01);
    delay_ms(2);
}