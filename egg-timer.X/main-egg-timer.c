#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#define _XTAL_FREQ 4000000

#define LED GP5
/////////////////////////////////////////////////////////configuraçôes//////////////////////////////////////////////////
#pragma config FOSC = INTRCIO   // Oscillator Selection bits (INTOSC oscillator: I/O function on GP4/OSC2/CLKOUT pin, I/O function on GP5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-Up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = OFF      // GP3/MCLR pin function select (GP3/MCLR pin function is digital I/O, MCLR internally tied to VDD)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config CP = OFF         // Code Protection bit (Program Memory code protection is disabled)
#pragma config CPD = OFF         // Data Code Protection bit (Data memory code protection is enabled)

long tempo_pisca=0;

void InitExternal_INT(void)
{
        LED = ~LED;
        //então: temos que 250khz / 250 incrementos = 1khz
        //então o valor do tmr0 será 256-250=6
        T0IF = 0;//  limpa flag de interrupção do timer0
        TMR0 = 6;//zera timer zero
 }


//////////////////////////////////////////////////////Rotina principal///////////////////////////////////////////////////////////////
main(void) {
    TRISIO = 0X01;//configura gp0 como entrada
    CMCON = 7;//desabilita comparadores
    ANSEL = 0;//habilita as portas para funcionar de forma digital
    WPU = 0X00;//desabilita pull ups
    TMR0 = 0;
    OSCCAL = 0XFF;//configura o oscilador interno para frequencia maxima(4mhz)
    OPTION_REG = 0X81;//pull up desabilitado/preescaler ligado ao timer0(dividido por 4)
    INTCON = 0XE0;//habilita interrupção do timer 0 e interrupção global e de perifericos
    //logo a frequencia de interrupção é 250khz
    for(;;)
    {
     //faz nada
    }//loop infinito

}