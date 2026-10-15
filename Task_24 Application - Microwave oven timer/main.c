#include <REG51.H>

/* ================= A2 BOARD PIN MAP =================
 * LCD1602 : D0-D7 = P0.0-P0.7, RS = P2.6, RW = P2.5, EN = P2.7
 * KEYPAD  : rows = P1.7-P1.4 (inputs), columns = P1.3-P1.0 (outputs)
 * BUZZER  : P2.5  (on A2 board buzzer SHARES P2.5 with LCD RW - this is
 *                  normal! LCD only reads RW while EN pulses, and we
 *                  never pulse EN during beeps, so both work fine)
 * RELAY   : external module, IN -> P3.7   (RELAY_ON/RELAY_OFF below)
 * DOOR    : external switch, P3.6 to GND  (0 = open, 1 = closed)
 * CRYSTAL : 12 MHz  -> 50 ms tick = 50000 counts -> reload 0x3CB0
 * ==================================================== */

sbit LCD_RS = P2^6;
sbit LCD_RW = P2^5;      /* same pin as buzzer on A2 - keep at 0 except beeps */
sbit LCD_EN = P2^7;

sbit BUZZ  = P2^5;       /* passive buzzer: needs toggling, not DC */

sbit RELAY = P3^7;       /* external relay module */
sbit DOOR  = P3^6;       /* door switch to GND: 0 = open, 1 = closed */

#define RELAY_ON   0     /* <-- active-LOW relay setting */
#define RELAY_OFF  1

/* ---------- Function Declarations ---------- */
void Beep(unsigned int duration);
void delay_ms(unsigned int ms);
void delay_500us(void);

/* ---------- FSM states ---------- */
enum { ST_IDLE, ST_SET, ST_RUNNING, ST_PAUSED, ST_DOOR_OPEN, ST_DONE };
unsigned char state = ST_IDLE;

unsigned int entry_val = 0;   /* digits typed, 123 means 1:23 */
unsigned int total_sec = 0;   /* cooking time in seconds      */

volatile unsigned char one_sec = 0;   /* set by Timer0 ISR */

/* ---------- software delays (12 MHz crystal) ---------- */
void delay_ms(unsigned int ms)      /* ~1 ms per unit */
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 110; j++);
}
void delay_500us(void)              /* half period for ~1 kHz buzzer tone */
{
    unsigned int j;
    for (j = 0; j < 250; j++);
}

/* ---------- buzzer (passive: toggle at ~1 kHz; restore pin to 0) ---------- */
void Beep(unsigned int duration)
{
    unsigned int i;
    for (i = 0; i < duration; i++)
    {
        BUZZ = 1; delay_500us();
        BUZZ = 0; delay_500us();
    }
    BUZZ = 0;                        /* back to LCD_RW = 0 (write mode) */
}

/* ================= LCD (HD44780, 8-bit, data on P0) ================= */
void lcd_cmd(unsigned char c)
{
    LCD_RS = 0; LCD_RW = 0; P0 = c;
    LCD_EN = 1; delay_ms(1); LCD_EN = 0; delay_ms(1);
}
void lcd_data(unsigned char d)
{
    LCD_RS = 1; LCD_RW = 0; P0 = d;
    LCD_EN = 1; delay_ms(1); LCD_EN = 0; delay_ms(1);
}
void lcd_init(void)
{
    delay_ms(20);        /* wait for LCD power-up */
    lcd_cmd(0x38);       /* 8-bit, 2 lines, 5x7 */
    lcd_cmd(0x0C);       /* display on, cursor off */
    lcd_cmd(0x06);       /* entry mode: increment */
    lcd_cmd(0x01);       /* clear */
    delay_ms(2);
}
void lcd_goto(unsigned char row, unsigned char col)
{
    lcd_cmd(row ? 0xC0 + col : 0x80 + col);
}
void lcd_str(char *s)
{
    while (*s) lcd_data(*s++);
}

/* ================= Timer0: 50 ms x 20 = 1 s (12 MHz) ================= */
void timer0_init(void)
{
    TMOD = (TMOD & 0xF0) | 0x01;  /* Timer0 mode 1, 16-bit */
    TH0  = 0x3C; TL0 = 0xB0;
    ET0 = 1; TR0 = 1; EA  = 1;
}
void t0_isr(void) interrupt 1
{
    static unsigned char n = 0;
    TH0 = 0x3C; TL0 = 0xB0;       /* reload 50 ms */
    if (++n >= 20) { n = 0; one_sec = 1; }
}

/* ================= Keypad Scan ================= */
unsigned char code KEYMAP[4][4] =
{   {'*','0','#','D'},
    {'7','8','9','C'},
    {'4','5','6','B'},
    {'1','2','3','A'}   };

unsigned char scan_keypad(void)
{
    unsigned char col, r;

    for (col = 0; col < 4; col++)
    {
        P1 = 0xFF;                 /* all high */
        P1 &= ~(0x08 >> col);      /* drive one column LOW */
        r = P1 & 0xF0;             /* read rows P1.7-P1.4 */
        if (r != 0xF0)
        {
            delay_ms(20);          /* debounce */
            r = P1 & 0xF0;
            if (r != 0xF0)
            {
                while ((P1 & 0xF0) != 0xF0);   /* wait for release */
                switch (r)
                {
                    case 0xE0: return KEYMAP[0][col];
                    case 0xD0: return KEYMAP[1][col];
                    case 0xB0: return KEYMAP[2][col];
                    case 0x70: return KEYMAP[3][col];
                    default:   return 0;
                }
            }
        }
    }
    return 0;
}

/* ================= Display helpers ================= */
void show_time(void)
{
    unsigned char m = total_sec / 60, s = total_sec % 60;
    char buf[6];
    buf[0] = '0' + m / 10;  buf[1] = '0' + m % 10;
    buf[2] = ':';
    buf[3] = '0' + s / 10;  buf[4] = '0' + s % 10;
    buf[5] = '\0';
    lcd_goto(0, 0);  lcd_str("Time:  ");
    lcd_str(buf);
}
void show_status(char *s)
{
    lcd_goto(1, 0);  lcd_str("                ");
    lcd_goto(1, 0);  lcd_str(s);
}

void enter_digit(unsigned char d)
{
    entry_val = entry_val * 10 + d;
    if (entry_val > 9959) entry_val = 9959;
    total_sec = (entry_val / 100) * 60 + (entry_val % 100);
    if (total_sec > 6039) total_sec = 6039;
}

void clear_all(void)
{
    entry_val = 0; total_sec = 0; RELAY = RELAY_OFF;
}

/* ================= Main FSM ================= */
void main(void)
{
    unsigned char key;

    RELAY = RELAY_OFF;
    BUZZ  = 0;               /* keeps LCD RW = 0 */
    lcd_init();
    timer0_init();
    lcd_str("MICROWAVE  v2.0 ");
    show_status("Set time, ON=S1 ");
    delay_ms(1500);
    lcd_cmd(0x01);
    show_time();
    show_status("ON=S1   CLR=S3  ");

    while (1)
    {
        /* ---- 1-second countdown tick ---- */
        if (one_sec)
        {
            one_sec = 0;
            if (state == ST_RUNNING)
            {
                if (total_sec > 0) total_sec--;
                if (total_sec == 0)
                {
                    state = ST_DONE; RELAY = RELAY_OFF;
                    show_status("TIME UP!        ");
                    Beep(500); delay_ms(200);
                    Beep(500); delay_ms(200);
                    Beep(500);
                }
            }
            else if (state == ST_DOOR_OPEN)
            {
                Beep(150);
            }
            show_time();
        }

        /* ---- door safety check ---- */
        if (DOOR == 0)
        {
            if (state == ST_RUNNING)
            {
                state = ST_DOOR_OPEN; RELAY = RELAY_OFF;
                show_status("DOOR OPEN! STOP ");
                Beep(300);
            }S
        }
        else if (state == ST_DOOR_OPEN)
        {
            show_status("Door OK. ON=S1  ");
        }

        /* ---- key event ---- */
        key = scan_keypad();
        if (key)
        {
            Beep(50);

            if (key >= '0' && key <= '9')
            {
                if (state == ST_IDLE || state == ST_SET)
                {
                    enter_digit(key - '0');
                    state = ST_SET;
                    show_time();
                }
            }
            else if (key == '#')              /* S3 = clear */
            {
                clear_all();
                state = ST_IDLE;
                show_time();
                show_status("Cleared         ");
            }
            else if (key == '*')              /* S1 = ON */
            {
                switch (state)
                {
                    case ST_SET:
                    case ST_IDLE:
                        if (total_sec > 0)
                        {
                            state = ST_RUNNING; RELAY = RELAY_ON;
                            show_status("COOKING...      ");
                        }
                        break;
                    case ST_RUNNING:
                        state = ST_PAUSED; RELAY = RELAY_OFF;
                        show_status("PAUSED          ");
                        break;
                    case ST_PAUSED:
                        state = ST_RUNNING; RELAY = RELAY_ON;
                        show_status("COOKING...      ");
                        break;
                    case ST_DOOR_OPEN:
                        if (DOOR == 1)
                        {
                            state = ST_RUNNING; RELAY = RELAY_ON;
                            show_status("COOKING...      ");
                        }
                        break;
                    case ST_DONE:
                        clear_all();
                        state = ST_IDLE;
                        show_time();
                        show_status("ON=S1   CLR=S3  ");
                        break;
                }
            }
        }
    }
}