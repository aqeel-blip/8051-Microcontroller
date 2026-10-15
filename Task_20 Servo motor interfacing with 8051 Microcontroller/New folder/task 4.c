#include <reg52.h>

// Hardware Pin Definitions
sbit SERVO_PIN = P1^0;  // PWM signal output to SG90
sbit K1        = P3^1;  // Button K1: Decrease angle
sbit K2        = P3^0;  // Button K2: Increase angle

// Onboard LCD1602 Pins (Hardwired via PCB traces)[cite: 1]
sbit RS = P2^6;  //[cite: 1]
sbit RW = P2^5;  //[cite: 1]
sbit EN = P2^7;  //[cite: 1]
#define LCD_DATA P0  // Port 0 data bus[cite: 1]

// Servo Control Variables
volatile unsigned char servo_position = 0; // Angle: 0 to 180 degrees
volatile bit pwm_state = 0;                // 0 = LOW state, 1 = HIGH state

// Dynamic Reload Times in Microseconds (12 MHz Crystal: 1 cycle = 1 us)
volatile unsigned int high_us = 500;        // Initial HIGH width (500 us for 0 deg)
volatile unsigned int low_us  = 19500;      // Initial LOW width (20000 - 500 us)

// Software Delay Function
void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 114; j++);
}

// Fixed LCD Command Writer
void lcd_cmd(unsigned char cmd) {
    RS = 0;
    RW = 0;
    LCD_DATA = cmd;
    EN = 1;
    delay_ms(1);
    EN = 0;
}

// Fixed LCD Data Writer
void lcd_data_write(unsigned char dat) {
    RS = 1;
    RW = 0;
    LCD_DATA = dat;
    EN = 1;
    delay_ms(1);
    EN = 0;
}

void lcd_init(void) {
    delay_ms(15);
    lcd_cmd(0x38); // 8-bit mode, 2 lines, 5x7 matrix
    lcd_cmd(0x0C); // Display ON, Cursor OFF
    lcd_cmd(0x06); // Auto-increment entry mode
    lcd_cmd(0x01); // Clear screen
    delay_ms(2);
}

void lcd_show_string(char *str) {
    while(*str) {
        lcd_data_write(*str++);
    }
}

void lcd_update_display(unsigned char angle) {
    lcd_cmd(0x80); // Line 1
    lcd_show_string("Servo Position ");
    
    lcd_cmd(0xC0); // Line 2
    lcd_show_string("Angle: ");
    lcd_data_write((angle / 100) + '0');        // Hundreds
    lcd_data_write(((angle / 10) % 10) + '0');  // Tens
    lcd_data_write((angle % 10) + '0');         // Units
    lcd_data_write(0xDF);                       // Degree symbol
    lcd_data_write(' ');
}

// Optimized Variable-Reload Timer 0 ISR (Fires only TWICE per 20ms frame)
void timer0_isr(void) interrupt 1 {
    unsigned int reload_val;
    
    if (pwm_state == 0) {
        SERVO_PIN = 1;
        pwm_state = 1;
        reload_val = 65536 - high_us;
    } else {
        SERVO_PIN = 0;
        pwm_state = 0;
        reload_val = 65536 - low_us;
    }
    
    TH0 = (unsigned char)(reload_val >> 8);
    TL0 = (unsigned char)(reload_val & 0xFF);
}

// Non-Blocking Key Scan Routine
void scan_keys(void) {
    // Key K1: Decrease Angle
    if (K1 == 0) {
        delay_ms(20); // Debounce delay
        if (K1 == 0) {
            if (servo_position > 0) {
                servo_position--;
                
                // Recalculate HIGH time (500us + angle * 11.111us) and LOW time
                high_us = 500 + (unsigned int)(servo_position * 11.111);
                low_us  = 20000 - high_us;
                
                lcd_update_display(servo_position);
            }
            delay_ms(100); // Continuous hold speed delay
        }
    }
    
    // Key K2: Increase Angle
    if (K2 == 0) {
        delay_ms(20); // Debounce delay
        if (K2 == 0) {
            if (servo_position < 180) {
                servo_position++;
                
                // Recalculate HIGH time (500us + angle * 11.111us) and LOW time
                high_us = 500 + (unsigned int)(servo_position * 11.111);
                low_us  = 20000 - high_us;
                
                lcd_update_display(servo_position);
            }
            delay_ms(100); // Continuous hold speed delay
        }
    }
}

void main(void) {
    // Timer 0 Setup: Mode 1 (16-Bit Timer)
    TMOD &= 0xF0;
    TMOD |= 0x01;
    
    // Compute initial timing for 0 degrees
    high_us = 500 + (unsigned int)(servo_position * 11.111);
    low_us  = 20000 - high_us;
    
    SERVO_PIN = 1;
    pwm_state = 1;
    
    // Load initial 16-bit counter value for HIGH pulse duration
    TH0 = (unsigned char)((65536 - high_us) >> 8);
    TL0 = (unsigned char)((65536 - high_us) & 0xFF);
    
    ET0 = 1; // Enable Timer 0 Interrupt
    EA  = 1; // Enable Global Interrupts
    TR0 = 1; // Start Timer 0
    
    lcd_init();
    lcd_update_display(servo_position);
    
    while (1) {
        scan_keys();
    }
}