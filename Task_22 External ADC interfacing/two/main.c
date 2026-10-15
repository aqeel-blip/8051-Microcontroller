#include <reg52.h>

// --- LCD1602 Pin Definitions (Prechin A2 Schematic) ---
#define LCD_PORT P0
sbit LCD_RS = P2^6;
sbit LCD_RW = P2^5;
sbit LCD_EN = P2^7;

// --- ET2046 / XPT2046 ADC Pin Definitions ---
sbit DOUT = P3^7;
sbit DIN  = P3^4;
sbit CS   = P3^5;
sbit DCLK = P3^6;

// --- Function Declarations ---
void DelayMs(unsigned int ms);
void LCD_WriteCmd(unsigned char cmd);
void LCD_WriteData(unsigned char dat);
void LCD_Init(void);
void LCD_ShowString(unsigned char row, unsigned char col, char *str);
unsigned int ET2046_ReadADC(unsigned char cmd);

// --- Delay Routine ---
void DelayMs(unsigned int ms) {
    unsigned int i, j;
    for(i = ms; i > 0; i--)
        for(j = 110; j > 0; j--);
}

// --- LCD1602 Driver Functions ---
void LCD_WriteCmd(unsigned char cmd) {
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_PORT = cmd;
    LCD_EN = 1;
    DelayMs(1);
    LCD_EN = 0;
}

void LCD_WriteData(unsigned char dat) {
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_PORT = dat;
    LCD_EN = 1;
    DelayMs(1);
    LCD_EN = 0;
}

void LCD_Init(void) {
    LCD_WriteCmd(0x38); // 8-bit mode, 2-line display, 5x7 matrix
    LCD_WriteCmd(0x0C); // Display ON, Cursor OFF
    LCD_WriteCmd(0x06); // Auto-increment cursor position
    LCD_WriteCmd(0x01); // Clear display screen
    DelayMs(2);
}

void LCD_ShowString(unsigned char row, unsigned char col, char *str) {
    if(row == 0) LCD_WriteCmd(0x80 + col);
    else if(row == 1) LCD_WriteCmd(0xC0 + col);
    
    while(*str) {
        LCD_WriteData(*str++);
    }
}

// --- ET2046 SPI Read Function ---
// --- Corrected ET2046 SPI Read Function ---
unsigned int ET2046_ReadADC(unsigned char cmd) {
    unsigned char i;
    unsigned int adc_value = 0;

    DCLK = 0;
    CS = 0; // Select ET2046

    // 1. Send 8-bit control word on DIN
    for(i = 0; i < 8; i++) {
        DIN = (cmd >> (7 - i)) & 0x01;
        DCLK = 1;
        DCLK = 0;
    }

    // 2. Clock cycle 9: Conversion sampling transition
    // Falling edge of Clock 9 outputs MSB (D11) on DOUT pin
    DCLK = 1;
    DCLK = 0;

    // 3. Read 12-bit ADC result from DOUT
    for(i = 0; i < 12; i++) {
        adc_value <<= 1;
        
        // Read current bit FIRST
        if(DOUT) {
            adc_value |= 0x01;
        }

        // Pulse DCLK to shift next bit out on DOUT for the next iteration
        DCLK = 1;
        DCLK = 0;
    }

    CS = 1; // Deselect ET2046
    return adc_value;
}

// --- Main Program Loop ---
void main(void) {
    unsigned int adc_val;
    unsigned int voltage_mv; // Voltage scaled (0 to 500 = 0.00V to 5.00V)

    LCD_Init();
    LCD_ShowString(0, 0, "Voltmeter AIN0:");

    while(1) {
        // Command 0x94: Single-ended 12-bit mode for channel AIN0 (Potentiometer AD1)
        adc_val = ET2046_ReadADC(0x94);

        // Convert 12-bit ADC value (0-4095) to Voltage (0-500)
        voltage_mv = (unsigned long)adc_val * 500 / 4095;

        // Display voltage on LCD Line 2
        LCD_WriteCmd(0xC0); // Move cursor to Line 2, Column 0
        LCD_ShowString(1, 0, "Voltage: ");
        
        LCD_WriteData('0' + (voltage_mv / 100));        // Units digit (0-5)
        LCD_WriteData('.');
        LCD_WriteData('0' + ((voltage_mv / 10) % 10));  // Tenths digit
        LCD_WriteData('0' + (voltage_mv % 10));         // Hundredths digit
        LCD_WriteData('V');
        LCD_ShowString(1, 14, "  ");

        DelayMs(200); // Update display every 200 ms
    }
}