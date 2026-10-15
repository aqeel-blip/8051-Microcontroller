#include <reg52.h>

// Pin definitions according to schematic
sbit K1 = P3^1;   // Button K1
sbit D1 = P2^0;   // LED D1

// Simple software delay function for debouncing (~10ms at 12MHz)
void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 120; j++);
    }
}

void main(void) {
    // Initial state: Turn OFF LED D1 (active low, so set to 1)
    D1 = 1;
    
    while (1) {
        // Step 1: Poll button state (active low: pressed == 0)
        if (K1 == 0) {
            // Debounce delay to ignore mechanical contact bounce
            delay_ms(10);
            
            // Step 2: Re-check if the button is still pressed
            if (K1 == 0) {
                // Toggle the LED state
                D1 = ~D1;
                
                // Step 3: Wait until the button is released to prevent multiple toggles
                while (K1 == 0);
            }
        }
    }
}