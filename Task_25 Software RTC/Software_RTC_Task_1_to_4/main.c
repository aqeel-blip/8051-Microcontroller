#include <reg51.h>
#include "lcd.h"

struct __time {
    unsigned int hour;
    unsigned int minute;
    unsigned int second;
    unsigned int day;
    unsigned int month;
    unsigned int year;
};

// Initial time set to current date and time
struct __time current_time = {11, 26, 24, 18, 9, 2026};

volatile bit second_flag = 0;
unsigned int timer_count = 0;

unsigned char is_leap_year(unsigned int year) {
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        return 1;
    }
    return 0;
}

unsigned char get_days_in_month(unsigned int month, unsigned int year) {
    if (month == 2) {
        return is_leap_year(year) ? 29 : 28;
    }
    if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    }
    return 31;
}

void update_time_structure(void) {
    current_time.second++;
    if (current_time.second >= 60) {
        current_time.second = 0;
        current_time.minute++;
        
        if (current_time.minute >= 60) {
            current_time.minute = 0;
            current_time.hour++;
            
            if (current_time.hour >= 24) {
                current_time.hour = 0;
                current_time.day++;
                
                if (current_time.day > get_days_in_month(current_time.month, current_time.year)) {
                    current_time.day = 1;
                    current_time.month++;
                    
                    if (current_time.month > 12) {
                        current_time.month = 1;
                        current_time.year++;
                    }
                }
            }
        }
    }
}

void update_lcd(void) {
    // Line 1: Time: HH:MM:SS
    lcd_set_cursor(0, 0);
    lcd_write_string("Time: ");
    lcd_write_2digit(current_time.hour);
    lcd_write_data(':');
    lcd_write_2digit(current_time.minute);
    lcd_write_data(':');
    lcd_write_2digit(current_time.second);

    // Line 2: Date: DD/MM/YYYY
    lcd_set_cursor(1, 0);
    lcd_write_string("Date: ");
    lcd_write_2digit(current_time.day);
    lcd_write_data('/');
    lcd_write_2digit(current_time.month);
    lcd_write_data('/');
    lcd_write_4digit(current_time.year);
}

// Timer 0 ISR (Vector 1)
void timer0_isr(void) interrupt 1 {
    // Reload for exact 50ms at 12 MHz
    TH0 = 0x3C;
    TL0 = 0xB0;
    
    timer_count++;
    if (timer_count >= 20) { // 20 * 50ms = 1 second
        timer_count = 0;
        second_flag = 1;
    }
}

void timer_init(void) {
    TMOD &= 0xF0; 
    TMOD |= 0x01; // Timer 0, Mode 1 (16-bit)
    
    TH0 = 0x3C;
    TL0 = 0xB0;
    
    ET0 = 1; // Enable Timer 0 Interrupt
    EA  = 1; // Enable Global Interrupts
    TR0 = 1; // Start Timer 0
}

void main(void) {
    lcd_init();
    lcd_write_string("RTC SOFTWARE");
    delay_ms(1000);
    lcd_clear();
    
    update_lcd(); // Initial render
    timer_init();

    while (1) {
        if (second_flag) {
            second_flag = 0;
            update_time_structure();
            update_lcd();
        }
    }
}