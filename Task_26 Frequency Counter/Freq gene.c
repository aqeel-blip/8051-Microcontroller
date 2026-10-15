#include <reg51.h>
#include "Freq gene.h"

/* 1 kHz: half-period ticks = TICK_HZ / (2 * GEN_HZ) = 460.8 -> 461 */
#define HALF_PERIOD_TICKS  (((TICK_HZ) + GEN_HZ) / (GEN_HZ * 2UL))
#define T0_RELOAD          (65536UL - HALF_PERIOD_TICKS)
#define T0_TH              ((unsigned char)(T0_RELOAD >> 8))
#define T0_TL              ((unsigned char)(T0_RELOAD & 0xFF))

sbit GEN_PIN = P1^0;

static void t0_reload(void) {
    TH0 = T0_TH;
    TL0 = T0_TL;
}

void timer0_isr(void) interrupt 1 {
    t0_reload();
    GEN_PIN = !GEN_PIN;
}

void freq_gen_init(void) {
    GEN_PIN = 0;
    /* Keep Timer 1 bits; only program Timer 0 mode 1 */
    TMOD = (TMOD & 0xF0) | 0x01;
    t0_reload();
    ET0 = 1;
    TR0 = 1;
}

unsigned int freq_gen_get_hz(void) {
    return (unsigned int)GEN_HZ;
}
