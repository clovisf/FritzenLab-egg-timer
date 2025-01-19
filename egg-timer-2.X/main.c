// reference: https://microcontroladores-c.blogspot.com/2014/09/como-usar-o-timer-e-o-comparador-de.html
// this made the interrupt work: https://stackoverflow.com/questions/73883808/pic16f877a-timer1-interrupt-time-is-not-as-expected
// some ideas aboout analog here: https://forum.microchip.com/s/topic/a5CV40000001m0zMAA/t397074
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#define _XTAL_FREQ 4000000

#define LED GP5
#define BUZZER GP2
//#define ANALOG_PIN GP0

/////////////////////////////////////////////////////////configuraçôes//////////////////////////////////////////////////
// CONFIG
#pragma config FOSC = INTRCIO
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-Up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF     // GP3/MCLR pin function select (GP3/MCLR pin function is digital I/O, MCLR internally tied to VDD)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config CP = OFF         // Code Protection bit (Program Memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)

long tempo_led=0;
int buttonpressed= 0;
//unsigned int Read_Adc(void);
int fortyms= 0;
int start= 0;
int stop= 0;
int adc_value= 0;

int Read_Adc(void) {
    ADCON0 |= 0x02; // Start conversion by setting GO bit
    __delay_us(5);
    while (ADCON0 & 0x02){}; // Wait for conversion to complete by checking GO bit

    return (ADRESH << 8) | ADRESL; // Combine high and low bytes
}

void __interrupt() ISR()//vetor de interrupção
 {
    if(T0IF)
    {   //então: temos que 250khz / 250 incrementos = 1khz
        //então o valor do tmr0 será 256-250=6   
        fortyms++;
        /*if(fortyms == 400 && start == 0){
            start= 1;
            fortyms= 0;
            
        }else if(fortyms == 400 && start == 1){
            start= 0;
            fortyms= 0;
        }*/
        if(start == 1){
            LED= 1;
        }else if(start== 0){
            LED= 0;
        }else{
            
        }
        T0IF = 0;//  limpa flag de interrupção do timer0
        TMR0 = 6;//zera timer zero
    }
    if(CMIF)
    { //tensão esta acima do esperado
         //LED = COUT;
         CMIF =0;//limpa flag de interrupção
    }
 }



//////////////////////////////////////////////////////Rotina principal///////////////////////////////////////////////////////////////
void main(void) {
    CMCON = 2;//habilita comparador para modo isolado
    ANSEL = 0b0010001; //Fosc/8 and AN0 as analog input
    ADCON0 = 0b10000001; // ADC enabled, select channel 0 (AN0/GP0), GO/DONE cleared
    WPU = 0X00;//desabilita pull ups
    TMR0 = 0;
    OSCCAL = 0XFF;//configura o oscilador interno para frequencia maxima(4mhz)
    OPTION_REG = 0X81;//pull up desabilitado/preescaler ligado ao timer0(dividido por 4)
    INTCON = 0XE0;//habilita interrupção do timer 0 e interrupção global e de perifericos
    CMIE = 1;//habilita interrupção do comparador
    TRISIO = 0X03;//configura gp0 e gp1 como entrada
    
    //logo a frequencia de interrupção é 250khz
    for(;;)
    {
       if(fortyms == 40){
           fortyms= 0;
           __delay_us(5);
           adc_value = Read_Adc();
           __delay_us(5);
           if(adc_value > 300 && adc_value < 450){ //0b100101100 and 0b111000010 in binary
              //stop 
               start= 0;
               stop= 1;
           }else if(adc_value > 451){ // 0b111000011 in binary
               //start
               stop= 0;
               start=1;
           }else{
               
           }
       }
    }//loop infinito

}