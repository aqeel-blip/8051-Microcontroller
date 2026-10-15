#include <reg51.h>
#include "lcd.h"

sbit RS = P2^6;
sbit RW = P2^5;
sbit EN = P2^7;

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 123; j++);
}

void lcd_write_byte(unsigned char dataa, unsigned char rs) {
    RS = rs;
    RW = 0;
    port0 = dataa;
    EN = 1;
    delay_ms(1);
    EN = 0;
    delay_ms(1);
}

void lcd_write_command(unsigned char command) {
    lcd_write_byte(command, 0);
}

void lcd_write_data(unsigned char dataa) {
    lcd_write_byte(dataa, 1);
}

void lcd_clear(void) {
    lcd_write_command(LCD_CLEAR_DISPLAY);
    delay_ms(2); // HD44780 requires >1.52ms execution time for clear
}

void lcd_init(void) {
    delay_ms(20);               // Wait for VCC to stabilize >15ms
    
    // HD44780 Initialization Sequence
    lcd_write_command(0x30);
    delay_ms(5);
    lcd_write_command(0x30);
    delay_ms(1);
    lcd_write_command(0x30);
    delay_ms(1);
    
    lcd_write_command(LCD_FUNCTION_SET); // 8-bit, 2 lines, 5x8 font
    delay_ms(1);
    lcd_write_command(LCD_DISPLAY_ON);   // Display ON, Cursor OFF
    delay_ms(1);
    lcd_write_command(LCD_ENTRY_MODE_SET); // Increment cursor automatically
    delay_ms(1);
    lcd_clear();
}

void lcd_set_cursor(unsigned char row, unsigned char col) {
    unsigned char address = (row == 0) ? (LCD_SET_DDRAM_ADDRESS | col) : (LCD_SET_DDRAM_ADDRESS | (0x40 + col));
    lcd_write_command(address);
}

void lcd_write_string(char *str) {
    while (*str) {
        lcd_write_data(*str++);
    }
}

// Lightweight 2-digit numerical display (replaces sprintf %02u)
void lcd_write_2digit(unsigned int val) {
    lcd_write_data((val / 10) + '0');
    lcd_write_data((val % 10) + '0');
}

// Lightweight 4-digit numerical display (replaces sprintf %04u)
void lcd_write_4digit(unsigned int val) {
    lcd_write_data((val / 1000) + '0');
    lcd_write_data(((val / 100) % 10) + '0');
    lcd_write_data(((val / 10) % 10) + '0');
    lcd_write_data((val % 10) + '0');
}