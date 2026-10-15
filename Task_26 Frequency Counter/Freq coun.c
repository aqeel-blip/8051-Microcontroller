#include <reg51.h>
#include "Freq gene.h"
#include "Freq coun.h"

volatile unsigned int period_ticks;
volatile bit period_ready;
volatile bit period_ovf;
volatile bit measuring;

void timer1_isr(void) interrupt 3 {
    TR1 = 0;
    measuring = 0;
    period_ovf = 1;
}

/*
 * 8051 INT0 is falling-edge when IT0=1. For a square wave the period
 * between consecutive edges equals rising-to-rising, so frequency matches.
 */
void int0_isr(void) interrupt 0 {
    unsigned int t;

    if (!measuring) {
        TL1 = 0;
        TH1 = 0;
        TF1 = 0;
        TR1 = 1;
        measuring = 1;
    } else {
        TR1 = 0;
        t = ((unsigned int)TH1 << 8) | TL1;
        TL1 = 0;
        TH1 = 0;
        TF1 = 0;
        TR1 = 1;
        period_ticks = t;
        period_ready = 1;
    }
}

void freq_count_init(void) {
    TMOD = (TMOD & 0x0F) | 0x10;   /* Timer 1 mode 1, 16-bit */
    TH1 = 0;
    TL1 = 0;
    IT0 = 1;
    PX0 = 1;
    EX0 = 1;
    ET1 = 1;
}

unsigned char freq_count_poll(unsigned int *hz) {
    unsigned int ticks;
    unsigned long f;

    if (period_ovf) {
        period_ovf = 0;
        return FREQ_OVF;
    }
    if (!period_ready)
        return FREQ_NONE;

    period_ready = 0;
    ticks = period_ticks;
    if (ticks < 2)
        return FREQ_OVF;

    f = TICK_HZ / ticks;
    if (f > 65535UL)
        return FREQ_OVF;

    *hz = (unsigned int)f;
    return FREQ_OK;
}
