// AD example from here: https://picguides.com/beginner/adc.php
// timer: https://microcontroladores-c.blogspot.com/2014/09/como-usar-o-timer-e-o-comparador-de.html

#include <xc.h>

#pragma config FOSC=INTRCIO, WDTE=OFF, PWRTE=OFF, MCLRE=ON, CP=OFF, \
                CPD=OFF, BOREN=OFF,

#define _XTAL_FREQ 4000000
#define LED GP5
#define LED2 GP2
#define ANALOG_PIN GP0
#define ANALOG_PIN2 GP1

unsigned int turnon = 0;
unsigned int analogvalue= 0;
unsigned int  cycle1= 0;


unsigned int Read_Adc() {
    ADCON0 |= 0x02; // Start conversion by setting GO bit
    __delay_us(5);
    while (ADCON0 & 0x02); // Wait for conversion to complete by checking GO bit

    return (ADRESH << 8) + ADRESL; // Combine high and low bytes
}
void init_ports(void) {
    ANSEL   = 0x00;  // Set ports as digital IO not analog input.
    ADCON0  = 0x00;  // Shut off ADC.
    CMCON   = 0x07;  // Shut off comparator.
    VRCON   = 0x00;  // Shut off the voltage reference.
    TRISIO  = 0x0;   // All GPIO pins output except 1&3.
    GPIO    = 0x00; // Make all pins LOW
    T0IE    = 0; // Disable timer interrupt.
    GPIE    = 0; // GPIO port change interrupt
    GIE     = 0; // Global interrupt for recognition of interrupts.
}
int main()
{
    init_ports();
    //ANSEL = 0x00;   //disable all analog ports
    //ANSELH = 0x00;

    //TRISIO= 0b111100;
    //ANSEL = 0b00101100; // Set GP0 as analog input, others as digital
    //CMCON = 0x07;  // Disable Comparators
    


   

	
    ///////////////////////
    // Main Program Loop //
    ///////////////////////
    while(1)
    {
        
        
           
        __delay_ms(200);
        LED= ~LED;
        //LED2= ~LED2;
            
        
            
                    
    }

    return 0;
}