#ifndef LCD_H
#define LCD_H

void lcd_init(void);
void lcd_clear(void);
void lcd_set_cursor(unsigned char row, unsigned char col);
void lcd_write_string(char *str);
void lcd_write_data(unsigned char dataa);
void lcd_write_uint(unsigned int val);
void lcd_pad_line(unsigned char used);
void delay_ms(unsigned int ms);

#endif
