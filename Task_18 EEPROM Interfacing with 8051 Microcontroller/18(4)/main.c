#include <reg52.h>
#include "eeprom.h"
#include "lcd1602.h"

#define EEPROM_COUNTER_ADDR 0x00

static unsigned char counter_value = 0;
static unsigned int sec_remaining = 0;

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = ms; i > 0; i--)
        for (j = 110; j > 0; j--);
}

void main(void) {
    // Initialize Peripherals
    LCD_Init();
    
    // Read saved counter value from EEPROM on startup
    counter_value = EEPROM_ReadByte(EEPROM_COUNTER_ADDR);
    
    // Static UI Display Setup
    LCD_ShowString(1, 1, "Count: ");
    LCD_ShowString(2, 1, "Next Save: ");

    while (1) {
        // Increment application counter
        counter_value++;
        
        // Update Counter Display on Line 1
        LCD_ShowNum(1, 8, counter_value, 3);
        
        // Wait 316 seconds (to achieve 10-year lifespan limit) while updating LCD countdown[cite: 2]
        for (sec_remaining = EEPROM_WRITE_INTERVAL_SEC; sec_remaining > 0; sec_remaining--) {
            LCD_ShowNum(2, 12, sec_remaining, 3);
            LCD_ShowString(2, 15, "s");
            delay_ms(1000);
        }
        
        // Write updated value to EEPROM after 316-second delay[cite: 2]
        EEPROM_WriteByte(EEPROM_COUNTER_ADDR, counter_value);
    }
}