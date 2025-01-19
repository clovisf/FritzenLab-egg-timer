// https://microcontroladores-c.blogspot.com/2014/09/como-usar-o-timer-e-o-comparador-de.html
// 

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#define _XTAL_FREQ 4000000

#define Buzzer GP2
#define LED GP5
/////////////////////////////////////////////////////////configuraçôes//////////////////////////////////////////////////
// CONFIG
#pragma config FOSC = INTRCIO
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-Up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF     // GP3/MCLR pin function select (GP3/MCLR pin function is digital I/O, MCLR internally tied to VDD)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config CP = OFF         // Code Protection bit (Program Memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)

long largura_pulso=0;
long quantida_de_passos=0;
long tempo_pisca=0;

void Interrupt()//vetor de interrupção
{   
    if(T0IF==1){
        tempo_pisca++;
        if(tempo_pisca > 10000)//aprox 1 segundo
        {
            GP5 = ~GP5;
            tempo_pisca=0;
            T0IF= 0;
        } 
        
        TMR0 = 231;//zera timer zero
        
    }
    
}
    




//////////////////////////////////////////////////////Rotina principal///////////////////////////////////////////////////////////////
void main(void) {
    //CMCON = 2;//habilita comparador para modo isolado
    ANSEL = 0b0100001;//usa AN0 como entrada analogica
    WPU = 0X00;//desabilita pull ups
    TMR0 = 0;
    OSCCAL = 0XFF;//configura o oscilador interno para frequencia maxima(4mhz)
    OPTION_REG = 0b00000001;//pull up desabilitado/preescaler ligado ao timer0(dividido por 4)
    INTCON = 0b11100100;//habilita interrupção do timer 0 e interrupção global e de perifericos
    //CMIE = 1;//habilita interrupção do comparador
    TRISIO = 0b011011;//configura  como entrada
    //logo a frequencia de interrupção é 250khz
    for(;;)
    {
        Buzzer= 0;
    }//loop infinito

}