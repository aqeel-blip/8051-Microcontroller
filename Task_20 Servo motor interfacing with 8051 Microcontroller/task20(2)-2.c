#include <reg52.h>
#include <intrins.h>  // Required for _nop_() precise delay

sbit SERVO = P1^0;    // Servo Signal Pin
sbit K1    = P3^1;    // Button K1 Pin

// Calibrated 100-microsecond delay loop (@ 11.0592 MHz)
void delay_100us(unsigned char t) {
    unsigned char i, j;
    while (t--) {
        _nop_();
        i = 2;
        j = 38;
        do {
            while (--j);
        } while (--i);
    }
}

// Millisecond delay for button debouncing
void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = ms; i > 0; i--)
        for (j = 110; j > 0; j--);
}

void main(void) {
    // Ticks in 100us units:
    //  5 * 100us = 0.5ms (0°)
    // 10 * 100us = 1.0ms (45°)
    // 15 * 100us = 1.5ms (90°)
    // 20 * 100us = 2.0ms (135°)
    // 25 * 100us = 2.5ms (180°)
    code unsigned char pulse_ticks[] = {5, 10, 15, 20, 25};
    
    unsigned char angle_index = 0;
    unsigned char high_ticks = 5;
    unsigned char low_ticks = 195; // 200 - 5 = 195 (Total 20ms period)

    while (1) {
        // 1. HIGH Pulse
        SERVO = 1;
        delay_100us(high_ticks);

        // 2. LOW Pulse
        SERVO = 0;
        delay_100us(low_ticks);

        // 3. Non-blocking button check
        if (K1 == 0) {
            delay_ms(15); // Debounce
            if (K1 == 0) {
                // Advance angle
                angle_index = (angle_index + 1) % 5;
                high_ticks = pulse_ticks[angle_index];
                low_ticks = 200 - high_ticks; // Keep total cycle at 20ms (200 * 100us)

                // Maintain PWM timing while button is held down
                while (K1 == 0) {
                    SERVO = 1;
                    delay_100us(high_ticks);
                    SERVO = 0;
                    delay_100us(low_ticks);
                }
            }
        }
    }
}