#include <reg51.h>

// --- Hardware Pin Definitions ---
// LCD Pins
#define LCD_DATA P0
sbit RW = P2^5;
sbit RS = P2^6;
sbit EN = P2^7;

// Keypad Pins
sbit R0 = P1^0; sbit R1 = P1^1; sbit R2 = P1^2; sbit R3 = P1^3;
sbit C0 = P1^4; sbit C1 = P1^5; sbit C2 = P1^6; sbit C3 = P1^7;

// DS1302 RTC Pins
sbit RTC_IO  = P3^4;
sbit RTC_CE  = P3^5;
sbit RTC_CLK = P3^6;

// DS18B20 Temp Sensor Pin
sbit DQ = P3^7;

// --- Global Variables ---
unsigned char current_state = 0; // 0: Calc, 1: RTC, 2: Temp

// --- Utility & LCD Routines ---
void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 112; j++);
}

void lcd_cmd(unsigned char cmd) {
    LCD_DATA = cmd; RW = 0; RS = 0; EN = 1;
    delay_ms(2); EN = 0;
}

void lcd_char(unsigned char dat) {
    LCD_DATA = dat; RW = 0; RS = 1; EN = 1;
    delay_ms(2); EN = 0;
}

void lcd_init() {
    lcd_cmd(0x38); lcd_cmd(0x0C); lcd_cmd(0x01); lcd_cmd(0x06);
}

void lcd_str(unsigned char *s) {
    while(*s) lcd_char(*s++);
}

void lcd_set_cursor(unsigned char r, unsigned char c) {
    lcd_cmd((r == 0 ? 0x80 : 0xC0) + c);
}

void lcd_num(int num) {
    if(num < 0) { lcd_char('-'); num = -num; }
    if(num / 100) lcd_char((num / 100) + '0');
    if(num / 10 || num >= 100) lcd_char(((num / 10) % 10) + '0');
    lcd_char((num % 10) + '0');
}

// --- Keypad Functions ---
char scan_key() {
    char k = '\0';
    P1 = 0xFF;
    
    R0=0; R1=1; R2=1; R3=1;
    if(!C0) k='+'; if(!C1) k='-'; if(!C2) k='*'; if(!C3) k='/';
    
    R0=1; R1=0; R2=1; R3=1;
    if(!C0) k='='; if(!C1) k='3'; if(!C2) k='6'; if(!C3) k='9';
    
    R0=1; R1=1; R2=0; R3=1;
    if(!C0) k='0'; if(!C1) k='2'; if(!C2) k='5'; if(!C3) k='8';
    
    R0=1; R1=1; R2=1; R3=0;
    if(!C0) k='C'; if(!C1) k='1'; if(!C2) k='4'; if(!C3) k='7';
    
    if(k != '\0') delay_ms(200); // Key debounce
    return k;
}

// --- DS1302 RTC Routines ---
void rtc_write_byte(unsigned char dat) {
    unsigned char i;
    for(i=0; i<8; i++) {
        RTC_IO = dat & 0x01;
        RTC_CLK = 1; RTC_CLK = 0;
        dat >>= 1;
    }
}

unsigned char rtc_read_byte(unsigned char cmd) {
    unsigned char i, dat = 0;
    RTC_CE = 1;
    rtc_write_byte(cmd);
    for(i=0; i<8; i++) {
        dat >>= 1;
        if(RTC_IO) dat |= 0x80;
        RTC_CLK = 1; RTC_CLK = 0;
    }
    RTC_CE = 0;
    return dat;
}

void rtc_write_reg(unsigned char cmd, unsigned char val) {
    RTC_CE = 1; rtc_write_byte(cmd); rtc_write_byte(val); RTC_CE = 0;
}

void rtc_init() {
    RTC_CE = 0; RTC_CLK = 0;
    rtc_write_reg(0x8E, 0x00); // Disable write protect
    rtc_write_reg(0x80, 0x00); // Start clock (Sec=00)
    rtc_write_reg(0x82, 0x30); // Min=30
    rtc_write_reg(0x84, 0x12); // Hour=12
}

void rtc_display_bcd(unsigned char val) {
    lcd_char(((val >> 4) & 0x0F) + '0');
    lcd_char((val & 0x0F) + '0');
}

void run_rtc_mode() {
    unsigned char h = rtc_read_byte(0x85);
    unsigned char m = rtc_read_byte(0x83);
    unsigned char s = rtc_read_byte(0x81);
    
    lcd_set_cursor(0, 0); lcd_str("RTC CLOCK");
    lcd_set_cursor(1, 0); lcd_str("TIME: ");
    rtc_display_bcd(h); lcd_char(':');
    rtc_display_bcd(m); lcd_char(':');
    rtc_display_bcd(s);
}

// --- DS18B20 Temperature Routines ---
void ds18b20_reset() {
    DQ = 1; DQ = 0; delay_ms(1); DQ = 1; delay_ms(1);
}

void ds18b20_write(unsigned char dat) {
    unsigned char i;
    for(i=0; i<8; i++) {
        DQ = 0; DQ = dat & 0x01; delay_ms(1); DQ = 1;
        dat >>= 1;
    }
}

unsigned char ds18b20_read() {
    unsigned char i, dat = 0;
    for(i=0; i<8; i++) {
        DQ = 0; dat >>= 1; DQ = 1;
        if(DQ) dat |= 0x80;
        delay_ms(1);
    }
    return dat;
}

void run_temp_mode() {
    unsigned char lsb, msb;
    int temp;
    
    ds18b20_reset();
    ds18b20_write(0xCC); // Skip ROM
    ds18b20_write(0x44); // Convert Temp
    
    ds18b20_reset();
    ds18b20_write(0xCC);
    ds18b20_write(0xBE); // Read Scratchpad
    
    lsb = ds18b20_read();
    msb = ds18b20_read();
    temp = (msb << 8) | lsb;
    temp = temp >> 4; // Convert to Celsius
    
    lcd_set_cursor(0, 0); lcd_str("TEMP MONITOR");
    lcd_set_cursor(1, 0); lcd_str("TEMP: ");
    lcd_num(temp); lcd_str(" C  ");
}

// --- Calculator App ---
void run_calc_mode(char key) {
    static int num1 = 0, num2 = 0;
    static char op = 0;
    static unsigned char step = 0;
    
    if(step == 0) {
        lcd_set_cursor(0, 0); lcd_str("CALCULATOR");
        lcd_set_cursor(1, 0); lcd_str("Ready...        ");
        num1 = 0; num2 = 0; op = 0; step = 1;
    }
    
    if(key >= '0' && key <= '9') {
        if(op == 0) {
            num1 = (num1 * 10) + (key - '0');
            lcd_set_cursor(1, 0); lcd_num(num1);
        } else {
            num2 = (num2 * 10) + (key - '0');
            lcd_set_cursor(1, 5); lcd_num(num2);
        }
    } else if(key == '+' || key == '-' || key == '*' || key == '/') {
        op = key;
        lcd_set_cursor(1, 4); lcd_char(op);
    } else if(key == '=') {
        int res = 0;
        if(op == '+') res = num1 + num2;
        if(op == '-') res = num1 - num2;
        if(op == '*') res = num1 * num2;
        if(op == '/' && num2 != 0) res = num1 / num2;
        
        lcd_set_cursor(1, 8); lcd_char('=');
        lcd_num(res);
        step = 0; // Reset after equals
    }
}

// --- Main Application Loop ---
void main() {
    char key;
    lcd_init();
    rtc_init();
    lcd_cmd(0x01);
    
    while(1) {
        key = scan_key();
        
        // App Switcher: 'C' key cycles between apps
        if(key == 'C') {
            current_state = (current_state + 1) % 3;
            lcd_cmd(0x01); // Clear display on state change
            delay_ms(100);
            continue;
        }
        
        // State Machine Execution
        switch(current_state) {
            case 0: run_calc_mode(key); break;
            case 1: run_rtc_mode();     break;
            case 2: run_temp_mode();    break;
        }
        delay_ms(100);
    }
}