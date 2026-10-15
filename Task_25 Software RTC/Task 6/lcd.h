#ifndef LCD_H
#define LCD_H

#include <reg51.h>

#define port0 P0

void lcd_init(void);
void lcd_clear(void);
void lcd_set_cursor(unsigned char row, unsigned char col);
void lcd_write_string(char *str);
void lcd_write_data(unsigned char dataa);
void lcd_write_2digit(unsigned char val);
void delay_ms(unsigned int ms);

#endif
