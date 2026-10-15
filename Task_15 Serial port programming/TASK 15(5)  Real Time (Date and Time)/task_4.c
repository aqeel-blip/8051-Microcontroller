#include <reg51.h>
#include "lcd.h"

// Updated Global Time and Date variables (11:52:00 AM, August 20, 2026)
unsigned char hour = 11, minute = 52, second = 0;
unsigned char day = 20, month = 8, year = 26;

// Function to initialize UART and Timer 0
void System_Init() {
    TMOD = 0x21;  // Timer 1: Mode 2 (8-bit auto-reload for UART), Timer 0: Mode 1 (16-bit for Delay)
    TH1 = 0xFD;   // Baud rate 9600 @ 11.0592 MHz crystal
    SCON = 0x50;  // Serial Mode 1, 8-bit UART, Receiver enabled
    TR1 = 1;      // Start Timer 1
}

// Function to send single character via UART
void UART_Send(unsigned char dat) {
    SBUF = dat;
    while (TI == 0);
    TI = 0;
}

// Function to send string via UART
void UART_Send_String(const char *str) {
    while (*str) {
        UART_Send(*str++);
    }
}

// 1-Second Delay using Timer 0 (20 overflows x 50ms)
void delay_1sec(void) {
    unsigned char i;
    for (i = 0; i < 20; i++) {
        TH0 = 0x4B; // Load 16-bit timer value for 50ms (65536 - 46080 = 19456 / 0x4B00)
        TL0 = 0x00;
        TR0 = 1;    // Start Timer 0
        while (TF0 == 0);
        TR0 = 0;    // Stop Timer 0
        TF0 = 0;    // Clear interrupt flag
    }
}

// Real-Time Increment Logic
void update_time_and_date(void) {
    unsigned char max_days;

    second++;
    if (second >= 60) {
        second = 0;
        minute++;
        if (minute >= 60) {
            minute = 0;
            hour++;
            if (hour >= 24) {
                hour = 0;
                day++;

                // Determine days in current month
                if (month == 4 || month == 6 || month == 9 || month == 11) {
                    max_days = 30;
                } else if (month == 2) {
                    max_days = (year % 4 == 0) ? 29 : 28; // Leap year logic
                } else {
                    max_days = 31;
                }

                if (day > max_days) {
                    day = 1;
                    month++;
                    if (month > 12) {
                        month = 1;
                        year++;
                    }
                }
            }
        }
    }
}

// Display time on 16x2 LCD (Line 1)
void display_time(void) {
    lcd_cmd(0x80);
    show("Time: ");
    show_char(hour / 10 + '0');
    show_char(hour % 10 + '0');
    show_char(':');
    show_char(minute / 10 + '0');
    show_char(minute % 10 + '0');
    show_char(':');
    show_char(second / 10 + '0');
    show_char(second % 10 + '0');
}

// Display date on 16x2 LCD (Line 2)
void display_date(void) {
    lcd_cmd(0xC0);
    show("Date: ");
    show_char(day / 10 + '0');
    show_char(day % 10 + '0');
    show_char('/');
    show_char(month / 10 + '0');
    show_char(month % 10 + '0');
    show_char('/');
    show_char(year / 10 + '0');
    show_char(year % 10 + '0');
}

// Send combined Time and Date to Hercules Serial Terminal
void send_time_and_date_UART(void) {
    UART_Send_String("Time: ");
    UART_Send(hour / 10 + '0');
    UART_Send(hour % 10 + '0');
    UART_Send(':');
    UART_Send(minute / 10 + '0');
    UART_Send(minute % 10 + '0');
    UART_Send(':');
    UART_Send(second / 10 + '0');
    UART_Send(second % 10 + '0');

    UART_Send_String(" | Date: ");
    UART_Send(day / 10 + '0');
    UART_Send(day % 10 + '0');
    UART_Send('/');
    UART_Send(month / 10 + '0');
    UART_Send(month % 10 + '0');
    UART_Send('/');
    UART_Send(year / 10 + '0');
    UART_Send(year % 10 + '0');

    UART_Send_String("\r\n"); // Line break for Hercules Terminal
}

void main(void) {
    lcd_init();
    System_Init();

    while (1) {
        display_time();
        display_date();
        send_time_and_date_UART();

        delay_1sec();              // Wait for 1 second
        update_time_and_date();    // Advance clock counters
    }
}