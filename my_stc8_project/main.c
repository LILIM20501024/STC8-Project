#include <reg51.h>
#include <intrins.h>

// ==========================================
// 1. STC8 ?????
// ==========================================
sfr P0M0 = 0x94; sfr P0M1 = 0x93; 
sfr P1M0 = 0x92; sfr P1M1 = 0x91; 
sfr P2M0 = 0x96; sfr P2M1 = 0x95; 
sfr P3M0 = 0xB2; sfr P3M1 = 0xB1; 
sfr P6   = 0xE8; sfr P6M0 = 0xCC; sfr P6M1 = 0xCB; 
sfr P7   = 0xF8; sfr P7M0 = 0xF6; sfr P7M1 = 0xF7; 
sfr AUXR = 0x8E;

// ==========================================
// 2. ??????
// ==========================================
// --- LCD12864 ---
sbit LCD_RS = P3^5; 
sbit LCD_RW = P3^4;
sbit LCD_E  = P3^2;
#define LCD_DATA P6  

// --- ???? ---
sbit KEY1 = P3^7; // ????
sbit KEY2 = P3^6; // ????

// --- ??? ---
sbit LED1 = P2^6; 
sbit LED2 = P2^7; 
sbit LED3 = P7^2; 
sbit LED4 = P7^1; 

// --- ??? ---
sbit BEEP = P2^0; 
sbit BEEP_PWR = P1^5; // ?????/???

// ==========================================
// 3. ????
// ==========================================
bit flag_led = 1;  
bit flag_beep = 0; 
bit key1_old = 1; 
bit key2_old = 1; 

unsigned char hour = 8, min = 30, sec = 0;
unsigned int ms_count = 0; 
unsigned char display_buf[17];

// ==========================================
// 4. ????
// ==========================================
void Delay_ms(unsigned int ms) {
    unsigned int i, j;
    for(i = ms; i > 0; i--)
        for(j = 110; j > 0; j--);
}

void NumToStr(unsigned char num, unsigned char *str) {
    str[0] = num / 10 + '0';
    str[1] = num % 10 + '0';
}

// ==========================================
// 5. LCD12864 ??
// ==========================================
void LCD_WriteCmd(unsigned char cmd) {
    LCD_RS = 0; LCD_RW = 0; LCD_DATA = cmd;
    LCD_E = 1; _nop_(); _nop_(); LCD_E = 0; Delay_ms(2); 
}
void LCD_WriteData(unsigned char dat) {
    LCD_RS = 1; LCD_RW = 0; LCD_DATA = dat;
    LCD_E = 1; _nop_(); _nop_(); LCD_E = 0; Delay_ms(2); 
}
void LCD_Init(void) {
    Delay_ms(100); LCD_WriteCmd(0x30); LCD_WriteCmd(0x30);
    LCD_WriteCmd(0x0C); LCD_WriteCmd(0x01); Delay_ms(20);
}
void LCD_DispStr(unsigned char x, unsigned char y, unsigned char *str) {
    unsigned char addr;
    switch(y) {
        case 0: addr = 0x80 + x * 2; break; 
        case 1: addr = 0x90 + x * 2; break; 
        case 2: addr = 0x88 + x * 2; break; 
        case 3: addr = 0x98 + x * 2; break; 
        default: addr = 0x80; break;
    }
    LCD_WriteCmd(addr);
    while(*str != '\0') { LCD_WriteData(*str); str++; }
}

// ==========================================
// 6. ???0 (??)
// ==========================================
void Timer0_Init(void) {
    AUXR &= 0x7F; TMOD &= 0xF0; TMOD |= 0x01; 
    TL0 = 0x66; TH0 = 0xFC; 
    TF0 = 0; TR0 = 1; ET0 = 1; EA = 1; 
}
void Timer0_ISR() interrupt 1 {
    TL0 = 0x66; TH0 = 0xFC; 
    ms_count++;
    if(ms_count >= 1000) { 
        ms_count = 0;
        sec++;
        if(sec >= 60) { sec = 0; min++; }
        if(min >= 60) { min = 0; hour++; }
        if(hour >= 24) { hour = 0; }
    }
}

// ==========================================
// 7. ???
// ==========================================
void main(void) {
    // ??????? P2.0 ? P1.5 ???????,?????????
    P2M1 = 0x00; P2M0 = 0x01;  
    P1M1 = 0x00; P1M0 = 0x20;  
    
    P3M1 = 0xC0; P3M0 = 0x00; 
    P6M1 = 0x00; P6M0 = 0x00;  
    P7M1 = 0x00; P7M0 = 0x00;  
    
    BEEP = 1; BEEP_PWR = 1; 
    LED1 = 1; LED2 = 1; LED3 = 1; LED4 = 1; 
    
    LCD_Init();
    Timer0_Init(); 
    
    // ????
    LCD_DispStr(0, 0, "ID:950225333");
    LCD_DispStr(0, 1, "Name:lizixuan");
    LCD_DispStr(0, 3, "System Ready!   "); 
    
    // ????:???
    BEEP_PWR = 0; BEEP = 0; 
    Delay_ms(150); 
    BEEP_PWR = 1; BEEP = 1;
    
    while(1) {
        // ????
        if(KEY1 == 0 && key1_old == 1) { flag_led = ~flag_led; }
        key1_old = KEY1; 
        
        if(KEY2 == 0 && key2_old == 1) { flag_beep = ~flag_beep; }
        key2_old = KEY2; 
        
        // ?????
        if(flag_beep) { 
            BEEP_PWR = 0; BEEP = 0; 
        } else { 
            BEEP_PWR = 1; BEEP = 1; 
        }
        
        // ????
        display_buf[0] = 'T'; display_buf[1] = 'i'; display_buf[2] = 'm'; display_buf[3] = 'e'; display_buf[4] = ':'; display_buf[5] = ' ';
        NumToStr(hour, &display_buf[6]); display_buf[8] = ':';
        NumToStr(min, &display_buf[9]); display_buf[11] = ':';
        NumToStr(sec, &display_buf[12]); display_buf[14] = ' '; display_buf[15] = ' '; display_buf[16] = '\0';
        LCD_DispStr(0, 2, display_buf); 
        
        // ???
      if(flag_led) {
            LED1 = 0; LED2 = 1; LED3 = 1; LED4 = 1; Delay_ms(100);
            LED1 = 1; LED2 = 0; LED3 = 1; LED4 = 1; Delay_ms(100);
            LED1 = 1; LED2 = 1; LED3 = 0; LED4 = 1; Delay_ms(100);
            LED1 = 1; LED2 = 1; LED3 = 1; LED4 = 0; Delay_ms(100);
        } else {
            LED1 = 1; LED2 = 1; LED3 = 1; LED4 = 1; Delay_ms(100); 
        }
    }
}