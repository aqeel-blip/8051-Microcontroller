#include "lcd1602.h"

// Hardware Pin Definitions (A2 Development Board Schematic)
sbit LCD_RS = P2^6;
sbit LCD_RW = P2^5;
sbit LCD_EN = P2^7;
#define LCD_DATAPORT P0

static void LCD_Delay(unsigned int ms) {
    unsigned int i, j;
    for (i = ms; i > 0; i--)
        for (j = 110; j > 0; j--);
}

void LCD_WriteCommand(unsigned char Command) {
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_DATAPORT = Command;
    LCD_EN = 1;
    LCD_Delay(1);
    LCD_EN = 0;
    LCD_Delay(1);
}

void LCD_WriteData(unsigned char Data) {
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_DATAPORT = Data;
    LCD_EN = 1;
    LCD_Delay(1);
    LCD_EN = 0;
    LCD_Delay(1);
}

void LCD_Init(void) {
    LCD_WriteCommand(0x38); // 8-bit mode, 2 lines, 5x7 dots
    LCD_WriteCommand(0x0C); // Display ON, Cursor OFF
    LCD_WriteCommand(0x06); // Auto-increment cursor
    LCD_WriteCommand(0x01); // Clear screen
    LCD_Delay(2);
}

static void LCD_SetCursor(unsigned char Line, unsigned char Column) {
    if (Line == 1) {
        LCD_WriteCommand(0x80 | (Column - 1));
    } else if (Line == 2) {
        LCD_WriteCommand(0xC0 | (Column - 1));
    }
}

void LCD_ShowChar(unsigned char Line, unsigned char Column, char Char) {
    LCD_SetCursor(Line, Column);
    LCD_WriteData(Char);
}

void LCD_ShowString(unsigned char Line, unsigned char Column, char *String) {
    LCD_SetCursor(Line, Column);
    while (*String != '\0') {
        LCD_WriteData(*String);
        String++;
    }
}

static unsigned int LCD_Pow(unsigned int X, unsigned char Y) {
    unsigned int Result = 1;
    while (Y--) Result *= X;
    return Result;
}

void LCD_ShowNum(unsigned char Line, unsigned char Column, unsigned int Number, unsigned char Length) {
    unsigned char i;
    LCD_SetCursor(Line, Column);
    for (i = Length; i > 0; i--) {
        LCD_WriteData('0' + (Number / LCD_Pow(10, i - 1)) % 10);
    }
}