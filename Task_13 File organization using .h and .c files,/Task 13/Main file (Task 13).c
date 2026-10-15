#include <reg52.h>
#include "LCDa.h"

// Pin definition for LED D1 (Port 2 Pin 0)
sbit LED1 = P2^0; 

void turn_on_led(void) {
    LED1 = 0;  // Active-low: 0 turns ON LED
}

void turn_off_led(void) {
    LED1 = 1;  // High turns OFF LED
}

void main(void) {
    while (1) {
        if (LCD == 1) {
            turn_on_led();   // Solid ON
        } else {
            turn_off_led();  // Solid OFF
        }
    }
}