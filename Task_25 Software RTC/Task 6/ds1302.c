#include <reg51.h>
#include "ds1302.h"

sbit DS1302_CLK = P3^6;
sbit DS1302_IO  = P3^4;
sbit DS1302_CE  = P3^5;

#define BCD2DEC(b)  ((((b) >> 4) & 0x0F) * 10 + ((b) & 0x0F))
#define DEC2BCD(d)  ((((d) / 10) << 4) | ((d) % 10))

static void ds1302_wb(unsigned char d) {
    unsigned char i;
    for (i = 0; i < 8; i++) {
        DS1302_IO  = d & 0x01;
        DS1302_CLK = 1;
        DS1302_CLK = 0;
        d >>= 1;
    }
}

static unsigned char ds1302_rb(void) {
    unsigned char i, d = 0;
    DS1302_IO = 1;
    for (i = 0; i < 8; i++) {
        d >>= 1;
        if (DS1302_IO) d |= 0x80;
        DS1302_CLK = 1;
        DS1302_CLK = 0;
    }
    return d;
}

static void ds1302_start(void) {
    DS1302_CE  = 0;
    DS1302_CLK = 0;
    DS1302_CE  = 1;
}

static void ds1302_wreg(unsigned char addr, unsigned char dat) {
    ds1302_start();
    ds1302_wb(addr);
    ds1302_wb(dat);
    DS1302_CE = 0;
}

void ds1302_init(void) {
    EA = 0;
    ds1302_wreg(0x8E, 0x00);
    ds1302_start();
    ds1302_wb(0x81);
    if (ds1302_rb() & 0x80)
        ds1302_wreg(0x80, 0x00);
    DS1302_CE = 0;
    EA = 1;
}

void ds1302_set_time(struct __time *t) {
    EA = 0;
    ds1302_wreg(0x8E, 0x00);
    ds1302_start();
    ds1302_wb(0xBE);
    ds1302_wb(DEC2BCD(t->second));
    ds1302_wb(DEC2BCD(t->minute));
    ds1302_wb(DEC2BCD(t->hour));
    ds1302_wb(DEC2BCD(t->day));
    ds1302_wb(DEC2BCD(t->month));
    ds1302_wb(0x01);
    ds1302_wb(DEC2BCD(t->year));
    ds1302_wb(0x80);
    DS1302_CE = 0;
    EA = 1;
}

unsigned char ds1302_get_time(struct __time *t) {
    unsigned char b0, b1, b2, b3, b4, b5;
    unsigned char sec, min, hr, day, mon, yr;

    EA = 0;
    ds1302_start();
    ds1302_wb(0xBF);
    b0 = ds1302_rb();
    b1 = ds1302_rb();
    b2 = ds1302_rb();
    b3 = ds1302_rb();
    b4 = ds1302_rb();
    b5 = ds1302_rb();
    ds1302_rb();
    DS1302_CE = 0;
    EA = 1;

    if ((b0 & 0x70) > 0x50) return 0;
    if ((b1 & 0x70) > 0x50) return 0;
    if ((b2 & 0x30) > 0x20) return 0;

    sec = BCD2DEC(b0 & 0x7F);
    min = BCD2DEC(b1 & 0x7F);
    hr  = BCD2DEC(b2 & 0x3F);
    day = BCD2DEC(b3 & 0x3F);
    mon = BCD2DEC(b4 & 0x1F);
    yr  = BCD2DEC(b5);

    if (sec > 59 || min > 59 || hr > 23) return 0;
    if (day < 1 || day > 31 || mon < 1 || mon > 12) return 0;

    t->second = sec;
    t->minute = min;
    t->hour   = hr;
    t->day    = day;
    t->month  = mon;
    t->year   = yr;
    return 1;
}
