/*******************************************************************************
Program for, generate an interrupt after every 2.5 seconds.And toggle a led connected at GP0 pin of PIC PIC12F675 using PIC12F675
Program Written by_ khatus
MCU:PIC12F675; X-Tal: 8MHz(internal). Compiler: mikroC pro for PIC v7.6.0
Date: 10-6-2021;
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>

//#define LED GP5

unsigned int cnt;  // cnt varialbe declaration(global variable)

void __interrupt() ISR(void)
{
    
    if(TMR0IF == 1)// if TMR0IF bit is set
    {
        cnt++;     //increment the counter
        TMR0IF=0;// if TMR0IF bit is cleared
        TMR0 = 61; //preload timer0 with value

    }
}

void main(void)
{

    TRISIO= 0b011011; //set I/O
    //GP5 = 0x00;  //Initialyy all GPIO pin is HIGF
    cnt = 0;  // Initialize cnt
    OPTION_REG =0b10000111;
    TMR0 = 0;//Timer0 preload value was 61
    INTCON= 0b11100000;
    //GIE=1; //Enables all unmasked interrupts
    //T0IE=1; //Enables the TMR0 interrupt
    
    while(1) {
        if (cnt >= 100) // if count greater than or equal to 100
        {
            if(GPIObits.GP5 == 0){
                GPIObits.GP5= 1;
            }else{
                GPIObits.GP5= 0;
            }
            cnt = 0;// Reset cnt
        }
    }

}
