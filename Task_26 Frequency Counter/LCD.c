#include <reg51.h>
#include "LCD.h"

sbit RS = P2^6;
sbit RW = P2^5;
sbit EN = P2^7;

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 123; j++);
}

static void lcd_byte(unsigned char v, unsigned char rs) {
    unsigned char i;
    RS = rs;
    RW = 0;
    P0 = v;
    EN = 1;
    for (i = 0; i < 8; i++);
    EN = 0;
    for (i = 0; i < 80; i++);
    P0 = 0xFF;
}

void lcd_write_data(unsigned char dataa) {
    lcd_byte(dataa, 1);
}

void lcd_clear(void) {
    lcd_byte(0x01, 0);
    delay_ms(2);
}

void lcd_init(void) {
    delay_ms(20);
    lcd_byte(0x30, 0); delay_ms(5);
    lcd_byte(0x30, 0); delay_ms(1);
    lcd_byte(0x30, 0); delay_ms(1);
    lcd_byte(0x38, 0); delay_ms(1);   /* 8-bit, 2 lines, 5x8 */
    lcd_byte(0x0C, 0); delay_ms(1);
    lcd_byte(0x06, 0); delay_ms(1);
    lcd_clear();
}

void lcd_set_cursor(unsigned char row, unsigned char col) {
    lcd_byte((row ? 0xC0 : 0x80) | col, 0);
}

void lcd_write_string(char *str) {
    while (*str) lcd_write_data(*str++);
}

void lcd_write_uint(unsigned int val) {
    unsigned char d[5];
    unsigned char n;

    if (val == 0) {
        lcd_write_data('0');
        return;
    }
    n = 0;
    while (val) {
        d[n++] = (unsigned char)(val % 10) + '0';
        val /= 10;
    }
    while (n)
        lcd_write_data(d[--n]);
}

void lcd_pad_line(unsigned char used) {
    while (used < 16) {
        lcd_write_data(' ');
        used++;
    }
}
