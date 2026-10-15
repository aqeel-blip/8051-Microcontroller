#ifndef RTCC_H
#define RTCC_H

typedef struct {
    unsigned char seconds;
    unsigned char minutes;
    unsigned char hours;
} RTC_Time;

void RTC_Init(void);
void RTC_GetTime(RTC_Time *time);
void RTC_SetTime(unsigned char h, unsigned char m, unsigned char s);

#endif