/************************************************************
 * Task 5 - Software RTC with PC time-setting over Serial
 * Board : STC89C52 @ 11.0592 MHz (see note below if 12 MHz)
 * LCD   : Data=P0, RS=P2.6, RW=P2.5, EN=P2.7
 * UART  : P3.0(RXD), P3.1(TXD) via CH340 USB-TTL
 *
 * PC -> MCU command format (14 digits + Enter):
 *   HHMMSSDDMMYYYY
 *   e.g.  11262418092026<CR>   = 11:26:24 18/09/2026
 * Other commands:
 *   ?<CR>  -> MCU replies with current time
 ************************************************************/
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

struct __time current_time = {10, 5, 30, 22, 9, 2026};

volatile bit second_flag = 0;
unsigned int timer_count = 0;

/* ---------------- UART variables ---------------- */
volatile bit  rx_ready = 0;                 // a full command line arrived
unsigned char rx_buf[20];
unsigned char rx_index = 0;

/* ---------------- UART driver ---------------- */
void uart_init(void) {
    TMOD &= 0x0F;           // clear Timer1 field
    TMOD |= 0x20;           // Timer1, Mode 2 (8-bit auto reload)
    TH1  = 0xFD;            // 9600 baud @ 11.0592 MHz
    TL1  = 0xFD;            // NOTE: if your crystal is 12.000 MHz use
                            //       TH1=TL1=0xF3 and 2400 baud instead!
    SCON = 0x50;            // UART mode 1, REN=1
    TR1  = 1;
    ES   = 1;               // enable UART interrupt
    EA   = 1;
}

void uart_putc(char c) {
    SBUF = c;
    while (TI == 0);
    TI = 0;
}

void uart_puts(char *s) {
    while (*s) uart_putc(*s++);
}

void uart_isr(void) interrupt 4 {
    if (RI) {
        RI = 0;
        rx_buf[rx_index] = SBUF;
        if (rx_buf[rx_index] == '\r' || rx_buf[rx_index] == '\n') {
            rx_buf[rx_index] = 0;           // terminate string
            if (rx_index > 0) rx_ready = 1;
            rx_index = 0;
        } else {
            if (rx_index < sizeof(rx_buf) - 1) rx_index++;
        }
    }
    if (TI) TI = 0;
}

/* ---------------- command parser ---------------- */
/* expects exactly 14 digits: HHMMSSDDMMYYYY            */
bit parse_and_set_time(unsigned char *s) {
    unsigned char i;
    unsigned char v[14];

    for (i = 0; i < 14; i++)
        if (s[i] < '0' || s[i] > '9') return 0;

    for (i = 0; i < 14; i++) v[i] = s[i] - '0';

    current_time.hour   = v[0]*10 + v[1];
    current_time.minute = v[2]*10 + v[3];
    current_time.second = v[4]*10 + v[5];
    current_time.day    = v[6]*10 + v[7];
    current_time.month  = v[8]*10 + v[9];
    current_time.year   = v[10]*1000 + v[11]*100 + v[12]*10 + v[13];

    if (current_time.hour > 23 || current_time.minute > 59 ||
        current_time.second > 59 || current_time.day < 1   ||
        current_time.day > 31  || current_time.month < 1   ||
        current_time.month > 12) return 0;

    return 1;
}

void send_current_time(void) {
    uart_putc('T'); uart_putc('=');
    uart_putc(current_time.hour/10 + '0');   uart_putc(current_time.hour%10 + '0');
    uart_putc(':');
    uart_putc(current_time.minute/10 + '0'); uart_putc(current_time.minute%10 + '0');
    uart_putc(':');
    uart_putc(current_time.second/10 + '0'); uart_putc(current_time.second%10 + '0');
    uart_putc(' ');
    uart_putc(current_time.day/10 + '0');    uart_putc(current_time.day%10 + '0');
    uart_putc('/');
    uart_putc(current_time.month/10 + '0');  uart_putc(current_time.month%10 + '0');
    uart_putc('/');
    uart_putc(current_time.year/1000 + '0'); uart_putc((current_time.year/100)%10 + '0');
    uart_putc((current_time.year/10)%10 + '0'); uart_putc(current_time.year%10 + '0');
    uart_putc('\r'); uart_putc('\n');
}

/* ---------------- RTC helpers ---------------- */
unsigned char is_leap_year(unsigned int year) {
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) return 1;
    return 0;
}

unsigned char get_days_in_month(unsigned int month, unsigned int year) {
    if (month == 2) return is_leap_year(year) ? 29 : 28;
    if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
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
    lcd_set_cursor(0, 0);
    lcd_write_string("Time: ");
    lcd_write_2digit(current_time.hour);
    lcd_write_data(':');
    lcd_write_2digit(current_time.minute);
    lcd_write_data(':');
    lcd_write_2digit(current_time.second);

    lcd_set_cursor(1, 0);
    lcd_write_string("Date: ");
    lcd_write_2digit(current_time.day);
    lcd_write_data('/');
    lcd_write_2digit(current_time.month);
    lcd_write_data('/');
    lcd_write_4digit(current_time.year);
}

/* ---------------- Timer 0 : 1 second tick ---------------- */
void timer0_isr(void) interrupt 1 {
    TH0 = 0x3C;                 // 50 ms @ 12 MHz (1 us machine cycle)
    TL0 = 0xB0;
    timer_count++;
    if (timer_count >= 20) {    // 20 x 50 ms = 1 s
        timer_count = 0;
        second_flag = 1;
    }
}

void timer_init(void) {
    TMOD &= 0xF0;
    TMOD |= 0x01;               // Timer0 Mode1 16-bit
    TH0 = 0x3C;
    TL0 = 0xB0;
    ET0 = 1;
    EA  = 1;
    TR0 = 1;
}

/* ---------------- main ---------------- */
void main(void) {
    lcd_init();
    lcd_write_string("RTC + SERIAL");
    delay_ms(1000);
    lcd_clear();

    update_lcd();
    timer_init();
    uart_init();
    uart_puts("Ready. Send HHMMSSDDMMYYYY + Enter\r\n");

    while (1) {
        if (rx_ready) {
            rx_ready = 0;
            if (rx_buf[0] == '?') {
                send_current_time();
            } else if (parse_and_set_time(rx_buf)) {
                update_lcd();
                uart_puts("OK\r\n");
            } else {
                uart_puts("ERR - use HHMMSSDDMMYYYY\r\n");
            }
        }

        if (second_flag) {
            second_flag = 0;
            update_time_structure();
            update_lcd();
        }
    }
}
