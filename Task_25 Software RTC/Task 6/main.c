/* Task 6: LCD line1 = software RTC, line2 = DS1302
 * UART 9600: send HHMMSSDDMMYYYY + Enter  (sets BOTH)
 *            send ? + Enter               (prints both)
 */
#include <reg51.h>
#include "lcd.h"
#include "ds1302.h"

struct __time sw_time;
struct __time ds_time;

volatile bit second_flag;
unsigned char timer_count;

volatile bit rx_ready;
unsigned char rx_buf[16];
unsigned char rx_index;

void uart_putc(char c) {
    ES = 0;
    SBUF = c;
    while (!TI);
    TI = 0;
    ES = 1;
}

void uart_puts(char *s) {
    while (*s) uart_putc(*s++);
}

void uart_2dig(unsigned char v) {
    uart_putc(v / 10 + '0');
    uart_putc(v % 10 + '0');
}

void uart_isr(void) interrupt 4 {
    unsigned char c;
    if (RI) {
        RI = 0;
        c = SBUF;
        if (c == '\r' || c == '\n') {
            rx_buf[rx_index] = 0;
            if (rx_index) rx_ready = 1;
            rx_index = 0;
        } else if (rx_index < 15) {
            rx_buf[rx_index++] = c;
        }
    }
}

void tick_sw(void) {
    unsigned char dim;
    sw_time.second++;
    if (sw_time.second < 60) return;
    sw_time.second = 0;
    sw_time.minute++;
    if (sw_time.minute < 60) return;
    sw_time.minute = 0;
    sw_time.hour++;
    if (sw_time.hour < 24) return;
    sw_time.hour = 0;
    sw_time.day++;
    if (sw_time.month == 2)
        dim = (sw_time.year & 3) ? 28 : 29;
    else if (sw_time.month == 4 || sw_time.month == 6 ||
             sw_time.month == 9 || sw_time.month == 11)
        dim = 30;
    else
        dim = 31;
    if (sw_time.day <= dim) return;
    sw_time.day = 1;
    sw_time.month++;
    if (sw_time.month > 12) {
        sw_time.month = 1;
        sw_time.year++;
    }
}

void lcd_hhmmss(unsigned char row, char a, char b, struct __time *t) {
    unsigned char i;
    lcd_set_cursor(row, 0);
    lcd_write_data(a);
    lcd_write_data(b);
    lcd_write_data(' ');
    lcd_write_2digit(t->hour);
    lcd_write_data(':');
    lcd_write_2digit(t->minute);
    lcd_write_data(':');
    lcd_write_2digit(t->second);
    for (i = 0; i < 5; i++) lcd_write_data(' ');   /* fill rest of 16 cols */
}

void update_lcd(void) {
    lcd_hhmmss(0, 'S', 'W', &sw_time);
    ds1302_get_time(&ds_time);
    lcd_hhmmss(1, 'D', 'S', &ds_time);
}

bit set_from_serial(unsigned char *s) {
    unsigned char i, n, v[14];
    n = 0;
    for (i = 0; s[i] && n < 14; i++) {
        if (s[i] >= '0' && s[i] <= '9') v[n++] = s[i] - '0';
    }
    if (n != 14) return 0;
    sw_time.hour   = v[0]*10 + v[1];
    sw_time.minute = v[2]*10 + v[3];
    sw_time.second = v[4]*10 + v[5];
    sw_time.day    = v[6]*10 + v[7];
    sw_time.month  = v[8]*10 + v[9];
    sw_time.year   = v[12]*10 + v[13];          /* keep YY only */
    if (sw_time.hour > 23 || sw_time.minute > 59 || sw_time.second > 59 ||
        sw_time.day < 1 || sw_time.day > 31 ||
        sw_time.month < 1 || sw_time.month > 12) return 0;

    TR0 = 0;
    timer_count = 0;
    second_flag = 0;
    TH0 = 0x4C;
    TL0 = 0x00;
    ds1302_set_time(&sw_time);
    TR0 = 1;
    return 1;
}

void answer_query(void) {
    uart_puts("SW ");
    uart_2dig(sw_time.hour); uart_putc(':');
    uart_2dig(sw_time.minute); uart_putc(':');
    uart_2dig(sw_time.second);
    uart_puts(" DS ");
    ds1302_get_time(&ds_time);
    uart_2dig(ds_time.hour); uart_putc(':');
    uart_2dig(ds_time.minute); uart_putc(':');
    uart_2dig(ds_time.second);
    uart_puts("\r\n");
}

void timer0_isr(void) interrupt 1 {
    TH0 = 0x4C;
    TL0 = 0x00;
    timer_count++;
    if (timer_count >= 20) {
        timer_count = 0;
        second_flag = 1;
    }
}

void main(void) {
    TMOD = 0x21;            /* T1 mode2 + T0 mode1 */
    TH1 = 0xFD; TL1 = 0xFD; /* 9600 @ 11.0592 MHz */
    SCON = 0x50;
    TH0 = 0x4C; TL0 = 0x00; /* 50 ms @ 11.0592 MHz 12T */
    ET0 = 1; ES = 1; EA = 1;
    TR1 = 1;                    /* UART baud; start Timer0 after LCD/RTC init */

    lcd_init();
    lcd_set_cursor(0, 0); lcd_write_string("SW --:--:--     ");
    lcd_set_cursor(1, 0); lcd_write_string("DS --:--:--     ");

    ds1302_init();
    TH0 = 0x4C; TL0 = 0x00;
    timer_count = 0;
    second_flag = 0;
    TR0 = 1;
    uart_puts("HHMMSSDDMMYYYY\r\n");

    while (1) {
        if (rx_ready) {
            rx_ready = 0;
            if (rx_buf[0] == '?') {
                answer_query();
            } else if (set_from_serial(rx_buf)) {
                update_lcd();
                uart_puts("OK\r\n");
            } else {
                uart_puts("ERR\r\n");
            }
        }
        if (second_flag) {
            second_flag = 0;
            tick_sw();
            update_lcd();
        }
    }
}
