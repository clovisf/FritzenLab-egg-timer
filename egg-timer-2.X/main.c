// reference: https://microcontroladores-c.blogspot.com/2014/09/como-usar-o-timer-e-o-comparador-de.html
// this made the interrupt work: https://stackoverflow.com/questions/73883808/pic16f877a-timer1-interrupt-time-is-not-as-expected
// some ideas aboout analog here: https://forum.microchip.com/s/topic/a5CV40000001m0zMAA/t397074

// ADC values for buttons measured in 07/09/2025:
// Stop 0.32V (65) and Start 0.64V (130), in a 0-1023 scale

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
volatile int ledtimer= 0;
volatile int buttonstimer= 0;
volatile int start= 0;
volatile int startbutton= 0;
volatile unsigned int adc_value= 0;
volatile int canstartblinking= 0;
volatile int processbuttonclicks= 0;
volatile int buttonclicks= 0;
volatile int enterbuttontimercounter= 0;
volatile int buttontimercounter= 0;

int supercounter= 0;

/*unsigned int Read_Adc(void) {
    ADCON0 |= 0x02; // Start conversion by setting GO bit
    __delay_us(5);
    while (ADCON0 & 0x02){}; // Wait for conversion to complete by checking GO bit

    return (ADRESH << 8) | ADRESL; // Combine high and low bytes
}*/
unsigned int Read_Adc(void) {
    ADCON0bits.GO_nDONE = 1;               // Start conversion
    while (ADCON0bits.GO_nDONE);           // Wait for conversion to complete
    return ((unsigned int)ADRESH << 8) | ADRESL; // Return full 10-bit result
}

void __interrupt() ISR()//vetor de interrupção
 {
    if(T0IF)
    {   //então: temos que 250khz / 250 incrementos = 1khz
        //então o valor do tmr0 será 256-250=6   
        ledtimer++;
        buttonstimer++;
        
        
        if(ledtimer >= 200 && processbuttonclicks != 0 && canstartblinking == 1){
            processbuttonclicks--;
            if(start == 1){
                start= 0;
            }else{
              start= 1;  
            }            
            ledtimer= 0;
        }else if(processbuttonclicks <= 0 && canstartblinking == 1){
            processbuttonclicks= 0;
            canstartblinking= 0;
        }
        if(start == 1){
            LED= 1;
        }else if(start== 0){
            LED= 0;
        }else{
            
        }
        T0IF = 0;//  limpa flag de interrupção do timer0
        TMR0 = 6;//zera timer zero
    }
    /*if(CMIF)
    { //tensão esta acima do esperado
         //LED = COUT;
         CMIF =0;//limpa flag de interrupção
    }*/
 }



//////////////////////////////////////////////////////Rotina principal///////////////////////////////////////////////////////////////
void main(void) {
    CMCON = 0x07; // ? disables the comparator module
    ANSEL = 0b0010001; //Fosc/8 and AN0 as analog input
    ADCON0 = 0b10000001; // ADC enabled, select channel 0 (AN0/GP0), GO/DONE cleared
    WPU = 0X00;//desabilita pull ups
    TMR0 = 0;
    OSCCAL = 0XFF;//configura o oscilador interno para frequencia maxima(4mhz)
    OPTION_REG = 0X81;//pull up desabilitado/preescaler ligado ao timer0(dividido por 4)
    INTCON = 0XE0;//habilita interrupção do timer 0 e interrupção global e de perifericos
    //CMIE = 1;//habilita interrupção do comparador
    TRISIO = 0X03;//configura gp0 e gp1 como entrada
    
    //logo a frequencia de interrupção é 250khz
    for(;;)
    {
       if(buttonstimer >= 300){
           buttonstimer= 0;
           
           adc_value = Read_Adc(); // reads ADC analog value
           
           if(adc_value > 90 && adc_value <= 1023 && canstartblinking == 0){ 
               buttonclicks++; // keeps track of the button clicks
               if(buttonclicks == 1){ //the first time the button was clicked in 3s..
                   enterbuttontimercounter= 1; //set a flag
               }else if(buttonclicks > 4){ // if more than 4 clicks, limit to 4
                   buttonclicks= 4;
               }else{
                   
               }
           }  
           
           if(enterbuttontimercounter == 1){ // when the button was clicked at least once
               buttontimercounter++; // increment a variable that will "count" 3s
               if(buttontimercounter > 10){ // when the time is over 3s..
                   enterbuttontimercounter= 0; // zero the main if condition and
                   buttontimercounter= 0; // zero the 3s counter
                   processbuttonclicks= 2 * buttonclicks; // transfers the button clicks to another variable
                   buttonclicks= 0; //zero the button clicks
                   canstartblinking= 1; // only starts blinking after 3s
               }
           }
                     
            
               
           //adc_value <= 90 && adc_value > 20 // 0b111000011 in binary
           
        }
    }//loop infinito

}