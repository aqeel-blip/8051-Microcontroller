#ifndef DS1302_H
#define DS1302_H

struct __time {
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
    unsigned char day;
    unsigned char month;
    unsigned char year;     /* 00-99 */
};

void ds1302_init(void);
void ds1302_set_time(struct __time *t);
unsigned char ds1302_get_time(struct __time *t);

#endif
