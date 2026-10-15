#include <reg52.h>

sbit D1 = P2^0;   // LED D1 on P2.0 (Active Low)
sbit K1 = P3^2;   // Button K1 moved to P3.2 (INT0 Pin required for Hardware Interrupt)

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
}

// External Interrupt 0 Service Routine (Vector Address 0)
void External0_ISR(void) interrupt 0 {
    delay_ms(15);         // Debounce delay to ignore switch noise
    if (K1 == 0) {        // Confirm valid press
        D1 = ~D1;         // Toggle LED state
        while (K1 == 0);  // Wait for button release
    }
}

void main(void) {
    D1 = 1;     // Turn OFF LED initially

    // Configure External Interrupt 0
    IT0 = 1;    // Set INT0 to trigger on falling edge (High-to-Low transition)
    EX0 = 1;    // Enable External Interrupt 0
    EA  = 1;    // Enable Global Interrupts

    while (1) {
        // Main loop remains free; interrupt handles button presses automatically
    }
}