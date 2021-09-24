/*
 * File:   BLDC.c
 * Author: yasinsahin
 *
 * Created on 24 Mayis 2021 Pazar, 14:48
 */


#include <xc.h>
#include <stdlib.h> 
#include <stdint.h>
// CONFIG1
#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator: Crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)


#define _XTAL_FREQ 10000000

unsigned int lowbyte;
unsigned int highbyte;

void I2C_Init(const unsigned long);
void I2C_Ack(void);
void I2C_Nack(void);
char I2C_Yaz(unsigned char);
void I2C_Hazir(void);
char I2C_Oku(char);

/*   KOMUTASYON

 * IR2130 entegresi giri?indeki pinleri invert edildi?i için normalde yaz?lan 0, 1 olarak yaz?ld? , 1 de 0 olarak
 * 
 */
void AH_BL(){
    CCP1CON = 0; //PWM Kapal?
    PORTB = 0XDF; // B LOW pini aktif
    PSTRCON = 0X20; // PWM giri?i RB5'TE
    CCP1CON = 0X0C; // PWM etkin
}

void AH_CL(){
    PORTB=0XEF;
}
void BH_CL(){
    CCP1CON=0;
    // 0XEF ~= 0X10;
    PORTB = 0XEF;
    PSTRCON=0X10;
    CCP1CON = 0X0C; //PWM etkin
}
void BH_AL(){
    PORTB = 0XBF; // 0X40 RB6
}

void CH_AL(){
    CCP1CON = 0;
    PORTB=0XBF;
    PSTRCON=0X08; // PWM ç?k??? RB3 te (CH pini)
    CCP1CON = 0X0C; // PWM etkin
}

void CH_BL(){
    PORTB = 0X20;
}

void pwm_duty(uint16_t duty_cycle){
 CCP1CON = ((duty_cycle << 4) & 0x30) | 0x0C;
 CCPR1L  = duty_cycle >> 2;

}

void I2C_Init(const unsigned long baudRate) 
{
  TRISC3 = 1;  TRISC4 = 1;  
  
  SSPCON  = 0b00101000;   
  SSPCON2 = 0b00000000;   
  
  SSPADD = (_XTAL_FREQ/(4*baudRate*100))-1; 
  SSPSTAT = 0b00000000;    
}

char I2C_Basla(unsigned char enkoder_adres){
    SEN=1;
    while(SEN);
    SSPIF=0;
    if( !SSPSTATbits.S) return 0;
    return (I2C_Yaz(enkoder_adres));

}

char I2C_Yaz(unsigned char veri){
    SSPBUF = veri;
    I2C_Hazir();
    if(ACKSTAT)
        return 1;
    else return 2;

}

void I2C_Hazir(){
 while(!(SSPIF)) 
     ; // I2C konfigürasyonu bitene kadar bekle
    SSPIF = 0;
}


char I2C_Dur(){
    I2C_Hazir();
    PEN = 1;
    while(PEN);
    SSPIF = 0;
    if(!SSPSTATbits.P) return 0;

}

char I2C_Oku(char flag){
    int gelenVeri = 0;
    RCEN = 1;
    
    while(!SSPSTATbits.BF);
    gelenVeri = SSPBUF;
    
    if(flag == 0) I2C_Ack();
    else {
        I2C_Nack();
        I2C_Hazir();
        return(gelenVeri);
    }
}

void I2C_Ack(void){
    ACKDT = 0;  ACKEN = 1;
    while(ACKEN);
}

void I2C_Nack(void){
    ACKDT=1; 
    ACKEN=1;
    while(ACKEN);
}
void main(void) {
   
    I2C_Init(100);
    PORTB=0;
    TRISB=0;
    CCP1CON=0X0C; //PWM modülü için tek ç?k?? konfigürasyon
    CCPR1L=0; // Duty cycle oran?n?n soldan 8 bitini 0 yap
   
   // PWM frekans? = Fosc/{[(PR2) + 1] * 4 * (TMR2 Prescaler De?eri)} = 9.77 kHz
    
   TMR2IF = 0;  // Timer2 interrupt bayra??n? s?f?rla
   T2CON = 0x04;    // Timer2 modülünü aktif et, prescaler oran? = 1
   PR2   = 0xFF;    // PR2 de?eri= 255
 
    while(1)
    {
    
    
    }
}

void dereceOku(){
    I2C_Basla(0X36); //AS5600 enkoderinin I2C adresi
    I2C_Yaz(0X0D); // gelen aç? de?erinin ilk 8 biti (7:0)
        
}

