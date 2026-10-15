#ifndef LCD_H
#define LCD_H

#include <reg52.h>
#include <intrins.h>

// =========================================================
// 1. HARDWARE PIN MAPPINGS
// =========================================================
sbit RS = P2^6;   // LCD Register Select (P2.6)
sbit RW = P2^5;   // LCD Read/Write (P2.5)
sbit EN = P2^7;   // LCD Enable (P2.7)

#define LCD_DATA_PORT P0  // Data pins connected to P0.4 - P0.7 in 4-bit mode

// =========================================================
// 2. CUSTOM CHARACTER BITMAPS
// =========================================================
// Char 0: Heart
static unsigned char code heart[8] = { 0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00, 0x00 };
// Char 1: Bell
static unsigned char code bell[8] = { 0x04, 0x0E, 0x0E, 0x0E, 0x1F, 0x00, 0x04, 0x00 };
// Char 2: Space Invader / Alien
static unsigned char code invader[8] = { 0x0E, 0x1F, 0x15, 0x1F, 0x0A, 0x15, 0x00, 0x00 };
// Char 3: Tick / Checkmark
static unsigned char code tick[8] = { 0x00, 0x01, 0x03, 0x16, 0x1C, 0x08, 0x00, 0x00 };
// Char 4: Speaker / Volume
static unsigned char code speaker[8] = { 0x01, 0x03, 0x0F, 0x0F, 0x0F, 0x03, 0x01, 0x00 };
// Char 5: Music Note
static unsigned char code music[8] = { 0x02, 0x03, 0x02, 0x0E, 0x1E, 0x0C, 0x00, 0x00 };
// Char 6: Skull / Lightbulb
static unsigned char code skull[8] = { 0x00, 0x0E, 0x15, 0x1F, 0x0E, 0x0A, 0x00, 0x00 };
// Char 7: Lock
static unsigned char code lock[8] = { 0x0E, 0x11, 0x11, 0x1F, 0x1B, 0x1B, 0x1F, 0x00 };

// =========================================================
// 3. FUNCTION DECLARATIONS
// =========================================================
void delay_ms(unsigned int ms);
void delay_us(unsigned int us);
void lcd_cmd_nibble(unsigned char nibble);
void lcd_cmd(unsigned char cmd);
void lcd_data(unsigned char ch);
void lcd_string(char *str);
void lcd_init(void);
void lcd_create_char(unsigned char location, unsigned char *charmap);

#endif