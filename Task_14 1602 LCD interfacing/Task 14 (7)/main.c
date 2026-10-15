#include "LCD.h" // We include our custom library

// =========================================================
// 4. MAIN APPLICATION
// =========================================================
void main(void) {
    unsigned char i;

    // Initialize LCD in 4-Bit Mode
    lcd_init();

    // Load Custom Glyphs into CGRAM Locations 0 to 7
    lcd_create_char(0, heart);
    lcd_create_char(1, bell);
    lcd_create_char(2, invader);
    lcd_create_char(3, tick);
    lcd_create_char(4, speaker);
    lcd_create_char(5, music);
    lcd_create_char(6, skull);
    lcd_create_char(7, lock);

    // Print Line 1: Title
    lcd_cmd(0x80);              // Move to Line 1, Position 0
    lcd_string("Custom Character");

    // Print Line 2: Display all 8 custom characters spaced out
    lcd_cmd(0xC0);              // Move to Line 2, Position 0
    for (i = 0; i < 8; i++) {
        lcd_data(i);            // Print character from CGRAM address i
        lcd_data(' ');          // Space between characters
    }

    while (1) {
        // Main loop
    }
}