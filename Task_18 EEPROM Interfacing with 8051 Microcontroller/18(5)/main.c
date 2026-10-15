#include <reg52.h>
#include "lcd.h"
#include "lcd1602.h"
#include "i2c.h"

sbit K1 = P3^1; // Push button K1

#define EEPROM_START_ADDR 0x00

static void DelayMs(unsigned int ms) {
    unsigned int i, j;
    for (i = ms; i > 0; i--)
        for (j = 110; j > 0; j--);
}

void main(void) {
    unsigned long counter = 0;

    LCD_Init();
    LCD_ShowString(1, 1, "EEPROM 32bit CNT");

    // Read stored 32-bit counter from EEPROM upon power-up/reset
    counter = EEPROM_ReadCounter32(EEPROM_START_ADDR);
    LCD_ShowNum(2, 1, counter, 10);

    while (1) {
        // Detect button K1 press with debouncing
        if (K1 == 0) {
            DelayMs(20); // Debounce
            if (K1 == 0) {
                counter++; // Increment 32-bit counter
                
                // Write modified counter back into 4 adjacent EEPROM bytes
                EEPROM_WriteCounter32(EEPROM_START_ADDR, counter);
                
                // Re-read back to verify EEPROM persistence
                counter = EEPROM_ReadCounter32(EEPROM_START_ADDR);
                LCD_ShowNum(2, 1, counter, 10);

                while (K1 == 0); // Wait for key release
            }
        }
    }
}