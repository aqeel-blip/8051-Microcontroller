#include <reg52.h>

sbit SERVO_PIN = P1^0; // Connected to P10 pin header[cite: 1]

volatile unsigned int pwm_count = 0;
volatile unsigned int duty_cycle = 5; // Default 0.5 ms high pulse (0 deg)[cite: 2]

// Timer 0 Interrupt Service Routine (Fires every 100 us)[cite: 2]
void Timer0_ISR(void) interrupt 1 {
    // Reload registers for 100 us tick @ 11.0592 MHz crystal[cite: 1]
    TH0 = 0xFF;
    TL0 = 0xA7;

    pwm_count++;

    // Pulse state toggle
    if (pwm_count < duty_cycle) {
        SERVO_PIN = 1;
    } else {
        SERVO_PIN = 0;
    }

    // Reset counter after 200 ticks (200 * 100 us = 20 ms period / 50 Hz)[cite: 2]
    if (pwm_count >= 200) {
        pwm_count = 0;
    }
}

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 114; j++);
}

void main(void) {
    // Configure Timer 0 in 16-bit Mode 1
    TMOD &= 0xF0;
    TMOD |= 0x01;

    TH0 = 0xFF;
    TL0 = 0xA7;

    ET0 = 1; // Enable Timer 0 Interrupt
    EA  = 1; // Enable Global Interrupts
    TR0 = 1; // Start Timer 0

    while (1) {
        // Sweep 0 deg (duty=5 -> 0.5 ms) to 180 deg (duty=25 -> 2.5 ms)[cite: 2]
        for (duty_cycle = 5; duty_cycle <= 25; duty_cycle++) {
            delay_ms(80); // Step delay allowing shaft time to complete movement
        }

        // Sweep 180 deg (duty=25) back to 0 deg (duty=5)[cite: 2]
        for (duty_cycle = 25; duty_cycle >= 5; duty_cycle--) {
            delay_ms(80);
        }
    }
}