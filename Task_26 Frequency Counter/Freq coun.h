#ifndef FREQ_COUN_H
#define FREQ_COUN_H

#define FREQ_NONE  0
#define FREQ_OK    1
#define FREQ_OVF   2   /* Timer 1 overflow: signal too slow */

void freq_count_init(void);

/* 0 = no new sample, 1 = *hz valid, 2 = overflow / too fast */
unsigned char freq_count_poll(unsigned int *hz);

#endif
