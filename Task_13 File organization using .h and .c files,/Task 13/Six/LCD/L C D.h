#ifndef __LCD_H__
#define __LCD_H__

#include <reg52.h>
#include <intrins.h>

// Hardware Pin Mappings
sbit RS = P2^6;
sbit RW = P2^5;
sbit EN = P2^7;

#define LCD_DATA_PORT P0

// Function Prototypes
void delay_ms(unsigned int ms);
void delay_us(unsigned int us);
void lcd_cmd_nibble(unsigned char nibble);
void lcd_cmd(unsigned char cmd);
void lcd_data(unsigned char ch);
void lcd_string(char *str);
void lcd_init(void);
void lcd_create_char(unsigned char location, unsigned char *charmap);

#endif