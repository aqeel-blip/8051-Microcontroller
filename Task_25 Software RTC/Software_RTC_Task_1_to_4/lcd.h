#ifndef LCD_H
#define LCD_H

#include <reg51.h>

#define port0 P0

// Standard HD44780 LCD Commands
#define LCD_CLEAR_DISPLAY        0x01
#define LCD_RETURN_HOME          0x02
#define LCD_ENTRY_MODE_SET       0x06 // Increment cursor, no display shift
#define LCD_DISPLAY_ON           0x0C // Display ON, Cursor OFF, Blink OFF
#define LCD_FUNCTION_SET         0x38 // 8-bit mode, 2 lines, 5x8 font
#define LCD_SET_DDRAM_ADDRESS    0x80

// Function Prototypes
void lcd_init(void);
void lcd_clear(void);
void lcd_set_cursor(unsigned char row, unsigned char col);
void lcd_write_string(char *str);
void lcd_write_data(unsigned char dataa);     // Added missing prototype
void lcd_write_command(unsigned char command);  // Added
void lcd_write_2digit(unsigned int val);
void lcd_write_4digit(unsigned int val);
void delay_ms(unsigned int ms);

#endif