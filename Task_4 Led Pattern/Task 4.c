#include <reg51.h>

// Define individual port pins using sbit
sbit D1 = P2^0;
sbit D2 = P2^1;
sbit D3 = P2^2;
sbit D4 = P2^3;
sbit D5 = P2^4;
sbit D6 = P2^5;
sbit D7 = P2^6;
sbit D8 = P2^7;

void delay_ms(unsigned int ms) {
    unsigned int x, y;
    for (x = 0; x < ms; x++) {
        for (y = 0; y < 120; y++);
    }
}

void main(void) {
    while (1) {
        // Pattern 1: D1, D3, D5, D7 ON (0) | D2, D4, D6, D8 OFF (1)
        D1 = 0;
        D2 = 1;
        D3 = 0;
        D4 = 1;
        D5 = 0;
        D6 = 1;
        D7 = 0;
        D8 = 1;
        delay_ms(500);

        // Pattern 2: D1, D3, D5, D7 OFF (1) | D2, D4, D6, D8 ON (0)
        D1 = 1;
        D2 = 0;
        D3 = 1;
        D4 = 0;
        D5 = 1;
        D6 = 0;
        D7 = 1;
        D8 = 0;
        delay_ms(500);
    }
}