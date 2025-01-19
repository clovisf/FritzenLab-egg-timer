// PIC12F675 Configuration Bit Settings
// 'C' source line config statements

#include <xc.h>

/*define clock freq*/

#ifndef _XTAL_FREQ
  #define _XTAL_FREQ 4000000  // 4MHZ crystal
#endif


// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG
#pragma config FOSC = INTRCIO   // Oscillator Selection bits (INTOSC oscillator: I/O function on GP4/OSC2/CLKOUT pin, I/O function on GP5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-Up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON     // MCLR
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config CP = OFF         // Code Protection bit (Program Memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)

void main()
{
    GPIO0=0x00;                        // make all GPIO port output
    TRISIO=0x00;                     // TRISIO direction as output
    ADCON0=0x00;                // Internal ADC OFF
    ANSEL=0x00;                  // All Analog selections pins are assigned as digital I/O
    while(1)
    {
        GPIO0=1;
        GPIO1=0;
        // Make GPIO0 port high
        __delay_ms(500); 
        GPIO0=0;
        GPIO0=1;// Make GPIO0 port low
        __delay_ms(500);
    }

}