/*
 * Task 26: frequency counter (loopback) + LCD + 8-digit 7-seg
 *
 * Crystal: FOSC = 11059200UL (11.0592 MHz), 12T  — defined in Freq gene.h
 *
 * Jumper: P1.0 (J20 pin 8) ---> P3.2 / INT0 (J29 pin 6)
 * LCD1602: D0-D7=P0, RS=P2.6, RW=P2.5, EN=P2.7
 * 7-seg:   segments P0 (74HC245), digits P2.2/P2.3/P2.4 (74HC138)
 *          LED8..LED5 = generated Hz,  LED4..LED1 = counted Hz
 */

#include <reg51.h>
#include "LCD.h"
#include "Freq gene.h"
#include "Freq coun.h"

unsigned int gen_freq;
unsigned int cnt_freq;
bit cnt_err = 1;
bit cnt_ovf;
unsigned char lcd_hold;

unsigned char seg_buf[8];
code unsigned char SEG_NUM[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};
#define SEG_DASH  0x40
#define SEG_BLANK 0x00

static void digits_to_seg(unsigned char lsd, unsigned int val, bit err) {
    unsigned char i;

    if (err) {
        for (i = 0; i < 4; i++)
            seg_buf[lsd + i] = SEG_DASH;
        return;
    }
    for (i = 0; i < 4; i++) {
        seg_buf[lsd + i] = SEG_NUM[val % 10];
        val /= 10;
    }
}

static void refresh_seg_values(void) {
    unsigned int g, c;

    g = gen_freq;
    if (g > 9999) g = 9999;
    digits_to_seg(4, g, 0);
    c = cnt_freq;
    if (!cnt_err && c > 9999) c = 9999;
    digits_to_seg(0, c, cnt_err);
}

static void scan_seg(void) {
    unsigned char i, d;

    for (i = 0; i < 8; i++) {
        P0 = 0x00;
        P2 = (P2 & 0xE3) | ((i & 0x07) << 2);
        P0 = seg_buf[i];
        for (d = 0; d < 80; d++);
    }
    P0 = 0x00;
}

static unsigned char digits_of(unsigned int n) {
    unsigned char k = 1;
    while (n >= 10) {
        n /= 10;
        k++;
    }
    return k;
}

static void lcd_show(void) {
    unsigned char used;

    lcd_set_cursor(0, 0);
    lcd_write_string("GEN:");
    lcd_write_uint(gen_freq);
    lcd_write_string("Hz");
    used = 4 + digits_of(gen_freq) + 2;
    lcd_pad_line(used);

    lcd_set_cursor(1, 0);
    lcd_write_string("CNT:");
    if (cnt_ovf) {
        lcd_write_string("Error");
        lcd_pad_line(9);
    } else if (cnt_err) {
        lcd_write_string("--------");
        lcd_pad_line(12);
    } else {
        lcd_write_uint(cnt_freq);
        lcd_write_string("Hz");
        used = 4 + digits_of(cnt_freq) + 2;
        lcd_pad_line(used);
    }
}

static void apply_measurement(void) {
    unsigned int hz;
    unsigned char st;
    bit paint;

    st = freq_count_poll(&hz);
    if (st == FREQ_NONE)
        return;

    paint = 0;
    if (st == FREQ_OVF) {
        cnt_err = 1;
        cnt_ovf = 1;
        paint = 1;
    } else {
        cnt_freq = hz;
        cnt_err = 0;
        cnt_ovf = 0;
    }
    refresh_seg_values();
    lcd_hold++;
    if (paint || lcd_hold >= 200) {
        lcd_hold = 0;
        lcd_show();
    }
}

void main(void) {
    unsigned char i;

    TMOD = 0x00;
    freq_gen_init();
    freq_count_init();
    EA = 1;

    gen_freq = freq_gen_get_hz();
    for (i = 0; i < 8; i++)
        seg_buf[i] = SEG_BLANK;

    lcd_init();
    refresh_seg_values();
    lcd_show();

    while (1) {
        apply_measurement();
        scan_seg();
    }
}
