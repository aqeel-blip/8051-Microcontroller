#include "LCD.h"

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 300; j++);
    }
}

void delay_us(unsigned int us) {
    while (us--) {
        _nop_();
        _nop_();
    }
}

void lcd_cmd_nibble(unsigned char nibble) {
    RS = 0;
    RW = 0;
    LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | ((nibble & 0x0F) << 4);
    EN = 1; delay_us(10); EN = 0; delay_us(10);
}

void lcd_cmd(unsigned char cmd) {
    lcd_cmd_nibble(cmd >> 4);
    lcd_cmd_nibble(cmd);
    delay_ms(3);
}

void lcd_data(unsigned char ch) {
    RS = 1;
    RW = 0;
    LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (ch & 0xF0);
    EN = 1; delay_us(10); EN = 0;
    
    LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | ((ch & 0x0F) << 4);
    EN = 1; delay_us(10); EN = 0;
    
    delay_us(100);
}

void lcd_string(char *str) {
    while (*str) {
        lcd_data(*str++);
    }
}

void lcd_init(void) {
    delay_ms(50);
    
    // 4-Bit Initialization Sequence
    lcd_cmd_nibble(0x03); delay_ms(10);
    lcd_cmd_nibble(0x03); delay_ms(1);
    lcd_cmd_nibble(0x03); delay_ms(1);
    lcd_cmd_nibble(0x02); delay_ms(2);

    lcd_cmd(0x28); // 4-bit, 2 lines, 5x8 matrix
    lcd_cmd(0x0C); // Display ON, Cursor OFF
    lcd_cmd(0x01); // Clear Display
    delay_ms(5);
    lcd_cmd(0x06); // Entry Mode Increment
}

void lcd_create_char(unsigned char location, unsigned char *charmap) {
    unsigned char i;
    location &= 0x07;
    lcd_cmd(0x40 | (location << 3));
    for (i = 0; i < 8; i++) {
        lcd_data(charmap[i]);
    }
}