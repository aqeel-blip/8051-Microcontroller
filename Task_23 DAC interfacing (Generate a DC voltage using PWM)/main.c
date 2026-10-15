#include <reg52.h>

// --- Pin Definitions ---
sbit PWM_PIN = P2^1; 
sbit KEY1    = P3^1; 

// LCD1602 Pins
sbit LCD_RS = P2^6;
sbit LCD_RW = P2^5;
sbit LCD_EN = P2^7;
#define LCD_DATA P0

// XPT2046 Pins
sbit DCLK = P3^6;
sbit CS   = P3^5; 
sbit DIN  = P3^4;
sbit DOUT = P3^7;

// --- Global Variables ---
unsigned char pwm_duty = 50; 
unsigned char pwm_counter = 0;

// --- Delay Function ---
void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 110; j++);
}

// --- PWM Timer Interrupt ---
void Timer0_Init() {
    TMOD &= 0xF0;
    TMOD |= 0x02; // Mode 2, 8-bit auto reload
    TH0 = 156;    // ~100us tick
    TL0 = 156;
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

void Timer0_ISR() interrupt 1 {
    pwm_counter++;
    if (pwm_counter >= 100) pwm_counter = 0;
    
    if (pwm_counter < pwm_duty) PWM_PIN = 1;
    else PWM_PIN = 0;
}

// --- LCD Functions ---
void LCD_WriteCommand(unsigned char cmd) {
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_DATA = cmd;
    LCD_EN = 1;
    delay_ms(1);
    LCD_EN = 0;
}

void LCD_WriteData(unsigned char dat) {
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_DATA = dat;
    LCD_EN = 1;
    delay_ms(1);
    LCD_EN = 0;
}

void LCD_Init() {
    LCD_WriteCommand(0x38); // 8-bit data, 2 lines, 5x7 font
    LCD_WriteCommand(0x0C); // Display ON, Cursor OFF
    LCD_WriteCommand(0x06); // Entry mode auto increment
    LCD_WriteCommand(0x01); // Clear display
    delay_ms(5);
}

void LCD_PrintStr(unsigned char x, unsigned char y, char *str) {
    if (y == 0) LCD_WriteCommand(0x80 | x);
    else LCD_WriteCommand(0x80 | (x + 0x40));
    
    while (*str != '\0') {
        LCD_WriteData(*str++);
    }
}

// --- ADC Function ---
unsigned int Read_ADC(unsigned char cmd) {
    unsigned char i;
    unsigned int dat = 0;
    
    CS = 0; DCLK = 0;
    for(i=0; i<8; i++) {
        DIN = (cmd >> (7-i)) & 0x01;
        DCLK = 1; DCLK = 0;
    }
    DCLK = 1; DCLK = 0;
    for(i=0; i<12; i++) {
        DCLK = 1;
        dat <<= 1;
        if(DOUT) dat++;
        DCLK = 0;
    }
    CS = 1;
    return dat;
}

// --- Main Program ---
void main() {
    unsigned int adc_value;
    unsigned int voltage_mv;
    char buffer[16];
    
    Timer0_Init();
    LCD_Init();
    
    LCD_PrintStr(0, 0, "Duty:    %");
    LCD_PrintStr(0, 1, "Volt: .  V");

    while(1) {
        // 1. Handle Button Press to change PWM (Task 3 addition)
        if (KEY1 == 0) {
            delay_ms(20);
            if (KEY1 == 0) {
                pwm_duty += 10;
                if (pwm_duty > 100) pwm_duty = 0;
                while(KEY1 == 0); // Wait for release
            }
        }
        
        // Update Duty Cycle on LCD
        LCD_WriteCommand(0x80 | 6); // Position cursor
        LCD_WriteData((pwm_duty / 100) ? '1' : ' ');
        LCD_WriteData((pwm_duty % 100) / 10 + '0');
        LCD_WriteData((pwm_duty % 10) + '0');

        // 2. Read ADC on AIN3 (0xE4 command) (Task 4)
        adc_value = Read_ADC(0xE4); 
        
        // 3. Convert 0-4095 scale to 0-500 scale (representing 0.00V to 5.00V)
        voltage_mv = (unsigned int)(((float)adc_value * 500.0) / 4095.0);
        
        // Update Voltage on LCD
        LCD_WriteCommand(0x80 | (5 + 0x40)); // Position cursor on 2nd line
        LCD_WriteData((voltage_mv / 100) + '0');       // Whole volts
        // Note: The decimal point is already written during init
        LCD_WriteCommand(0x80 | (7 + 0x40)); 
        LCD_WriteData(((voltage_mv % 100) / 10) + '0'); // Tenths
        LCD_WriteData((voltage_mv % 10) + '0');         // Hundredths
        
        delay_ms(100); // Small delay to stop LCD flickering
    }
}