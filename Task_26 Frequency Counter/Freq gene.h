#ifndef FREQ_GENE_H
#define FREQ_GENE_H

/* Board crystal, 12T machine cycle. Timer tick = FOSC/12 = 921600 Hz. */
#define FOSC    11059200UL
#define TICK_HZ (FOSC / 12UL)
#define GEN_HZ  1000UL

/* Square wave on P1.0 (jumper to P3.2 / INT0). */
void freq_gen_init(void);
unsigned int freq_gen_get_hz(void);

#endif
