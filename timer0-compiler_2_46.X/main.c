/*
 Based on this code: https://microcontroladores-c.blogspot.com/2014/09/timer0-com-pic12f675.html
 */
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#define _XTAL_FREQ 4000000

#define LED GP5
#define Buzzer GP2
/////////////////////////////////////////////////////////configuraçôes//////////////////////////////////////////////////
#pragma config FOSC = INTRCIO   // Oscillator Selection bits (INTOSC oscillator: I/O function on GP4/OSC2/CLKOUT pin, I/O function on GP5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-Up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = OFF      // GP3/MCLR pin function select (GP3/MCLR pin function is digital I/O, MCLR internally tied to VDD)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config CP = OFF         // Code Protection bit (Program Memory code protection is disabled)
#pragma config CPD = OFF         // Data Code Protection bit (Data memory code protection is enabled)


volatile uint8_t counter= 0;
volatile uint16_t timerminutes= 0;
volatile int reading= 0;
volatile int voltageX10= 0;
void driveLED(int i);
volatile int enableCounter= 0;
volatile int activateBuzzer= 0;
volatile int buzzerCount= 0;

void __interrupt() isr()//interrupt vector, +/- 65ms (or 47,67ms?)
{   
    counter++;
    if(enableCounter == 1){
       timerminutes++; 
    }
    
    
    if(counter == 1){
        GO_nDONE= 1;        
        while(GO_nDONE);
        reading = ((ADRESH<<8)+ADRESL); 
        voltageX10= reading * 5; 
        
        if(voltageX10 > 500){
            driveLED(1);
            enableCounter= 1;
        }else if(voltageX10 > 100 && voltageX10 <= 500){
           driveLED(0);
           enableCounter= 0;
           if(Buzzer == 0){
            activateBuzzer= 0;
           }
           
        }else{
            
        }
        counter= 0;
        
            
        
    }
    if(timerminutes == 1258){
        driveLED(0);
        enableCounter= 0;
        timerminutes= 0;
        activateBuzzer= 1;
    }
    
    if(activateBuzzer == 1){
        buzzerCount++;
        if(buzzerCount < 15){
            Buzzer= 1;
        }else if(buzzerCount < 30){
            Buzzer= 0;
        }else{
            buzzerCount= 0;
        }
    }else{
        
    }
    TMR0IF = 0;//  clear timer0 interrupt flag
    TMR0 = 0;// zeroes timer 0 counting, so that it couts from 256 down to 0 again
}

void driveLED(int i){
    if(i == 0){
        LED= 0;
    }else if(i == 1){
        LED= 1;
    }else{
        
    }
}

//////////////////////////////////////////////////////Main routine///////////////////////////////////////////////////////////////
void main(void) {
    TRISIO = 0b00000001;// GP0/AN0 input
    CMCON = 7;// disable comparators
    ANSEL = 0b00010001; // AN0 as analog
    ADCON0=0b10000000; // right justified, VDD as ref, channel AN0
    ADON=1; // Enable ADC
    WPU = 0X00;// pull ups disabled
    TMR0 = 0;
    OSCCAL = 0b11111111;// internal oscillator to max frequency
    OPTION_REG = 0b10000111;
    INTCON = 0b11100000;
    
    Buzzer= 0;
    LED= 0;
    
    while(1)
    {
        
        
    }//infinite loop

}