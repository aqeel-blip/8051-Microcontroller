#include <reg52.h>
#include <string.h>
#include <stdio.h> 

sbit SERVO_PWM = P2^4;  
sbit BUZZER    = P2^5;  
sbit LCD_RS    = P2^6;
sbit LCD_RW    = P2^5;   
sbit LCD_E     = P2^7;
#define LCD_DATA P0     

typedef enum {
    STATE_ENTER_PASS,
    STATE_UNLOCK,
    STATE_CHANGE_REQ,
    STATE_NEW_PASS,
    STATE_DISABLED
} LockState;

LockState currentState;

char currentPassword[7] = "123456"; 
char inputBuffer[7];     
unsigned char inputIndex = 0;
unsigned char wrongAttempts = 0;
unsigned char changeAttempts = 0;

unsigned int keyHoldTime = 0;
bit oncPressed = 0;
bit lockTimerActive = 0;
unsigned int lockTimerCount = 0;

void DelayMs(unsigned int ms) {
    unsigned int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 120; j++);
}

void BuzzerBeep(unsigned int durationMs) {
    BUZZER = 0; 
    DelayMs(durationMs);
    BUZZER = 1; 
}

void TriggerContinuousAlarm() {
    while(1) {
        BUZZER = 0; 
    }
}

void UART_Init() {
    TMOD |= 0x20; 
    TH1 = 0xFD;   
    TR1 = 1;      
    SCON = 0x50;  
}

void UART_SendChar(char ch) {
    SBUF = ch;
    while(!TI);
    TI = 0;
}

void UART_SendString(char *str) {
    while(*str) {
        UART_SendChar(*str++);
    }
}

void LCD_Command(unsigned char cmd) {
    LCD_RS = 0; 
    LCD_RW = 0;         
    DelayMs(1);
    LCD_DATA = cmd;     
    DelayMs(1);
    LCD_E = 1; 
    DelayMs(3);         
    LCD_E = 0;
    DelayMs(3);         
}

void LCD_WriteData(unsigned char dat) {
    LCD_RS = 1; 
    LCD_RW = 0;         
    DelayMs(1);
    LCD_DATA = dat;     
    DelayMs(1);
    LCD_E = 1; 
    DelayMs(3); 
    LCD_E = 0;
    DelayMs(3);
}

void LCD_Init() {
    DelayMs(100);       
    
    LCD_Command(0x30);
    DelayMs(10);
    LCD_Command(0x30);
    DelayMs(1);
    LCD_Command(0x30);
    DelayMs(1);
    
    LCD_Command(0x38);  
    DelayMs(2);
    LCD_Command(0x0C);  
    DelayMs(2);
    LCD_Command(0x06);  
    DelayMs(2);
    LCD_Command(0x01);  
    DelayMs(20);        
}

void LCD_DisplayString(unsigned char row, unsigned char col, char *str) {
    unsigned char addr = (row == 0) ? (0x80 + col) : (0xC0 + col);
    LCD_Command(addr);
    while(*str) {
        LCD_WriteData(*str++);
    }
}

void SetServoAngle(unsigned char angle) {
    unsigned int i;
    unsigned int highTimeMs = (angle == 180) ? 2 : 0; 
    unsigned int lowTimeMs = 20 - highTimeMs;

    for(i = 0; i < 20; i++) { 
        SERVO_PWM = 1;
        if(highTimeMs > 0) {
            DelayMs(highTimeMs);
        } else {
            unsigned int k;
            for(k = 0; k < 60; k++); 
        }
        
        SERVO_PWM = 0;
        DelayMs(lowTimeMs);
    }
}

char Keypad_Scan() {
    unsigned char keyValue = '\0';
    P1 = 0x7F; 
    if (P1 != 0x7F) {
        DelayMs(20); 
        if (P1 != 0x7F) {
            switch(P1 & 0x0F) {
                case 0x0E: keyValue = '1'; break; 
                case 0x0D: keyValue = '2'; break; 
                case 0x0B: keyValue = '3'; break; 
                case 0x07: keyValue = 'S'; break; 
            }
            while((P1 & 0x0F) != 0x0F); 
            return keyValue;
        }
    }
    P1 = 0xBF;
    if (P1 != 0xBF) {
        DelayMs(20);
        if (P1 != 0xBF) {
            switch(P1 & 0x0F) {
                case 0x0E: keyValue = '4'; break; 
                case 0x0D: keyValue = '5'; break; 
                case 0x0B: keyValue = '6'; break; 
                case 0x07: keyValue = 'B'; break; 
            }
            while((P1 & 0x0F) != 0x0F);
            return keyValue;
        }
    }

    P1 = 0xDF; 
    if (P1 != 0xDF) {
        DelayMs(20);
        if (P1 != 0xDF) {
            switch(P1 & 0x0F) {
                case 0x0E: keyValue = '7'; break; 
                case 0x0D: keyValue = '8'; break; 
                case 0x0B: keyValue = '9'; break; 
                case 0x07: keyValue = 'C'; break; 
            }
            while((P1 & 0x0F) != 0x0F);
            return keyValue;
        }
    }

    P1 = 0xEF; 
    if (P1 != 0xEF) {
        DelayMs(20);
        if (P1 != 0xEF) {
            switch(P1 & 0x0F) {
                case 0x0E: keyValue = '0'; break; 
                case 0x0D: keyValue = 'D'; break; 
                case 0x0B: keyValue = 'E'; break; 
                case 0x07: keyValue = 'F'; break; 
            }
            if (keyValue == 'F') {
                return 'F'; 
            }
            while((P1 & 0x0F) != 0x0F);
            return keyValue;
        }
    }

    return '\0'; 
}

void main() {
    char key;
    char lcdBuf[16];
    
    DelayMs(200); 

    P0 = 0xFF; 
    P1 = 0xFF; 
    BUZZER = 1; 
    SERVO_PWM = 0; 
    
    UART_Init();
    LCD_Init();
    
    LCD_DisplayString(0, 0, "System Booting...");
    DelayMs(1000);
    LCD_Command(0x01); 

    UART_SendString("DEFAULT PASS: ");
    UART_SendString(currentPassword);
 
    SetServoAngle(0);

    currentState = STATE_ENTER_PASS;
    LCD_DisplayString(0, 0, "Enter password");

    while(1) {
        key = Keypad_Scan();
        
        if (currentState == STATE_DISABLED) {
            LCD_Command(0x01);
            LCD_DisplayString(0, 0, "Door disabled");
            TriggerContinuousAlarm();
        }

        switch(currentState) {
                
            case STATE_ENTER_PASS:
                if (key == 'F') { 
                    if (!oncPressed) { 
                        oncPressed = 1; 
                        keyHoldTime = 0; 
                    } else {
                        keyHoldTime++;
                        DelayMs(5);
                        if (keyHoldTime >= 200) { 
                            oncPressed = 0;
                            inputIndex = 0;
                            memset(inputBuffer, 0, sizeof(inputBuffer));
                            LCD_Command(0x01);
                            LCD_DisplayString(0, 0, "Enter old pass");
                            currentState = STATE_CHANGE_REQ;
                        }
                    }
                } else {
                    if (oncPressed && key == '\0') {
                        oncPressed = 0;
                        inputIndex = 0;
                        memset(inputBuffer, 0, sizeof(inputBuffer));
                        LCD_Command(0x01);
                        LCD_DisplayString(0, 0, "Enter password");
                    }
                }
                
                if (key >= '0' && key <= '9') {
                    if (inputIndex < 6) {
                        inputBuffer[inputIndex++] = key;
                        inputBuffer[inputIndex] = '\0';
                        LCD_DisplayString(1, 0, inputBuffer);
                        BuzzerBeep(100);
                    }
                }
                
                if (key == 'S') {
                    if (inputIndex == 6) {
                        if (strcmp(inputBuffer, currentPassword) == 0) {
                            wrongAttempts = 0;
                            LCD_Command(0x01);
                            SetServoAngle(180);
                            BuzzerBeep(300);
                            
                            lockTimerCount = 5;
                            lockTimerActive = 1;
                            currentState = STATE_UNLOCK;
                        } else {
                            wrongAttempts++;
                            LCD_Command(0x01);
                            sprintf(lcdBuf, "W%bu", wrongAttempts);
                            LCD_DisplayString(0, 0, lcdBuf);
                            BuzzerBeep(1000);
                            
                            if (wrongAttempts >= 5) {
                                currentState = STATE_DISABLED;
                            } else {
                                DelayMs(1500);
                                inputIndex = 0;
                                memset(inputBuffer, 0, sizeof(inputBuffer));
                                LCD_Command(0x01);
                                LCD_DisplayString(0, 0, "Enter password");
                            }
                        }
                    } else {
                        BuzzerBeep(80); DelayMs(50); BuzzerBeep(80);
                    }
                }
                break;
                
            case STATE_UNLOCK:
                if (lockTimerActive) {
                    sprintf(lcdBuf, "Door Unlocked %u", lockTimerCount);
                    LCD_DisplayString(0, 0, lcdBuf);
                    DelayMs(1000);
                    lockTimerCount--;
                    if (lockTimerCount == 0) {
                        lockTimerActive = 0;
                        
                        // Time elapsed: Safely drive the servo mechanism back to 0 Degrees to relock
                        SetServoAngle(0);
                        
                        inputIndex = 0;
                        memset(inputBuffer, 0, sizeof(inputBuffer));
                        LCD_Command(0x01);
                        LCD_DisplayString(0, 0, "Enter password");
                        currentState = STATE_ENTER_PASS;
                    }
                }
                break;
                
            case STATE_CHANGE_REQ:
                sprintf(lcdBuf, "C%bu", changeAttempts);
                LCD_DisplayString(0, 14, lcdBuf);
                
                if (key >= '0' && key <= '9') {
                    if (inputIndex < 6) {
                        inputBuffer[inputIndex++] = key;
                        inputBuffer[inputIndex] = '\0';
                        LCD_DisplayString(1, 0, inputBuffer);
                        BuzzerBeep(100);
                    }
                }
                
                if (key == 'S') {
                    if (inputIndex == 6) {
                        if (strcmp(inputBuffer, currentPassword) == 0) {
                            changeAttempts = 0;
                            inputIndex = 0;
                            memset(inputBuffer, 0, sizeof(inputBuffer));
                            LCD_Command(0x01);
                            LCD_DisplayString(0, 0, "Enter new pass");
                            currentState = STATE_NEW_PASS;
                        } else {
                            changeAttempts++;
                            if (changeAttempts >= 3) {
                                currentState = STATE_DISABLED;
                            } else {
                                BuzzerBeep(150); DelayMs(100);
                                BuzzerBeep(150); DelayMs(100);
                                BuzzerBeep(150);
                                inputIndex = 0;
                                memset(inputBuffer, 0, sizeof(inputBuffer));
                                LCD_Command(0x01);
                                LCD_DisplayString(0, 0, "Enter password");
                                currentState = STATE_ENTER_PASS;
                            }
                        }
                    }
                }
                break;
                
            case STATE_NEW_PASS:
                if (key >= '0' && key <= '9') {
                    if (inputIndex < 6) {
                        inputBuffer[inputIndex++] = key;
                        inputBuffer[inputIndex] = '\0';
                        LCD_DisplayString(1, 0, inputBuffer);
                        BuzzerBeep(100);
                    }
                }
                
                if (key == 'S') {
                    if (inputIndex == 6) {
                        DelayMs(500);
                        strcpy(currentPassword, inputBuffer);
                        BuzzerBeep(500); 
                        inputIndex = 0;
                        memset(inputBuffer, 0, sizeof(inputBuffer));
                        LCD_Command(0x01);
                        LCD_DisplayString(0, 0, "Enter password");
                        currentState = STATE_ENTER_PASS;
                    }
                }
                break;
        }
    }
}