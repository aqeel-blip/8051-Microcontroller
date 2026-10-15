#include <reg52.h>
#include "LCD.h"

#define MAX_LEN 20

// Global RX Buffer and Flags
char rx_buffer[MAX_LEN + 1];
unsigned char rx_index = 0;
bit msg_received = 0;

// Time and Date Numerical Variables
unsigned char hour = 1, min = 41, sec = 0;
unsigned char day = 18, month = 4;
unsigned int year = 2018;

// UART Peripheral Initialization
void UART_Init(void) {
    SCON = 0x50;  // 8-bit UART, enable receiver
    TMOD |= 0x20; // Timer 1, Mode 2 (8-bit auto-reload)
    TH1  = 0xFD;  // 9600 Baud rate @ 11.0592 MHz
    TL1  = 0xFD;
    TR1  = 1;     // Start Timer 1
    ES   = 1;     // Enable Serial Interrupt
    EA   = 1;     // Enable Global Interrupt
}

void UART_SendChar(char ch) {
    SBUF = ch;
    while (TI == 0);
    TI = 0;
}

void UART_SendString(char *str) {
    while (*str) UART_SendChar(*str++);
}

// UART Interrupt Handler
void UART_ISR(void) interrupt 4 {
    if (RI) {
        char ch = SBUF;
        RI = 0;
        
        if (!msg_received) {
            if (rx_index == 0 && (ch == '\n' || ch == '\r')) return;
            
            if (ch == '\n' || ch == '\r') {
                rx_buffer[rx_index] = '\0';
                msg_received = 1;
            } else {
                rx_buffer[rx_index++] = ch;
                if (rx_index >= MAX_LEN) {
                    rx_buffer[MAX_LEN] = '\0';
                    msg_received = 1;
                }
            }
        }
    }
}

// Function to update LCD display matching image layout
void Update_LCD_Display(void) {
    // Line 1: Time: hh:mm:ss
    LCD_SetCursor(0, 0);
    LCD_String("Time: ");
    LCD_PrintTwoDigits(hour);
    LCD_Char(':');
    LCD_PrintTwoDigits(min);
    LCD_Char(':');
    LCD_PrintTwoDigits(sec);
    
    // Line 2: Date: dd.mm.yyyy
    LCD_SetCursor(1, 0);
    LCD_String("Date: ");
    LCD_PrintTwoDigits(day);
    LCD_Char('.');
    LCD_PrintTwoDigits(month);
    LCD_Char('.');
    LCD_PrintTwoDigits(year / 100);
    LCD_PrintTwoDigits(year % 100);
}

// Parsing function for Time or Date format
void Process_Received_Data(void) {
    // 1. Time string in format "hh:mm:ss" or "T:hh:mm:ss"
    if (rx_buffer[2] == ':' && rx_buffer[5] == ':') {
        hour = (rx_buffer[0] - '0') * 10 + (rx_buffer[1] - '0');
        min  = (rx_buffer[3] - '0') * 10 + (rx_buffer[4] - '0');
        sec  = (rx_buffer[6] - '0') * 10 + (rx_buffer[7] - '0');
        UART_SendString("Time Updated!\r\n");
    }
    // 2. Date string in format "dd.mm.yyyy" or "D:dd.mm.yyyy"
    else if (rx_buffer[2] == '.' && rx_buffer[5] == '.') {
        day   = (rx_buffer[0] - '0') * 10 + (rx_buffer[1] - '0');
        month = (rx_buffer[3] - '0') * 10 + (rx_buffer[4] - '0');
        year  = (rx_buffer[6] - '0') * 1000 + (rx_buffer[7] - '0') * 100 +
                (rx_buffer[8] - '0') * 10   + (rx_buffer[9] - '0');
        UART_SendString("Date Updated!\r\n");
    } 
    else {
        UART_SendString("Invalid Format. Send hh:mm:ss\\n\\r or dd.mm.yyyy\\n\\r\r\n");
    }
}

void main(void) {
    LCD_Init();
    UART_Init();
    
    // Show initial default display
    Update_LCD_Display();

    while (1) {
        if (msg_received) {
            Process_Received_Data();
            Update_LCD_Display();
            
            // Reset buffer flags
            rx_index = 0;
            msg_received = 0;
        }
    }
}