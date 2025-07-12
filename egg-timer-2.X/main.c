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
#pragma config FOSC = INTRCIO   // Oscillator Selection bits (INTOSC oscillator: I/O function on GP4/OSC2/CLKOUT pin, I/O function on GP5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-Up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // GP3/MCLR pin function select (GP3/MCLR pin function is digital I/O, MCLR internally tied to VDD)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config CP = OFF         // Code Protection bit (Program Memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)

// Global variables for timer and state management
long tempo_led = 0;
int buttonpressed = 0;
volatile int ledtimer = 0;
volatile int buttonstimer = 0;
volatile unsigned char start = 0;
volatile unsigned char startbutton = 0;
volatile unsigned int adc_value = 0;
volatile unsigned char canstartblinking = 0;
volatile int processbuttonclicks = 0;
volatile int buttonclicks = 0;
volatile unsigned char enterbuttontimercounter = 0;
volatile int buttontimercounter = 0;
volatile unsigned char starttimer = 0;
volatile int counttime = 0;
int supercounter = 0;
volatile unsigned char timecontrol = 0;
volatile int finalquantity = 2000; // Default time for 1 click (2 seconds at 1ms/count)
volatile unsigned char finalbuzzer = 0;      // Flag to activate the final buzzer sequence
volatile int finalbuzzercounter = 0; // Counter for buzzer duration
volatile unsigned char buzzeron = 0;         // Flag to control buzzer output
volatile unsigned char processstarted = 0;   // Flag to indicate if a timing process was initiated
volatile int longtimecounter= 0;

// Function to read ADC value from AN0 (GP0)
unsigned int Read_Adc(void) {
    ADCON0bits.GO_nDONE = 1;          // Start A/D conversion
    while (ADCON0bits.GO_nDONE);      // Wait for conversion to complete
    // Return full 10-bit result (ADRESH is 8-bit, ADRESL is 2-bit, shifted to form 10-bit)
    return ((unsigned int)ADRESH << 8) | ADRESL;
}

// Interrupt Service Routine (ISR)
void __interrupt() ISR() {
    // Check if Timer0 interrupt flag is set
    if (T0IF) {
        // Timer0 is configured to interrupt every 1ms (250kHz / 250 increments = 1kHz)
        // TMR0 is reloaded with 6 (256-250) to achieve 250 counts per interrupt
        ledtimer++;
        buttonstimer++;

        // Main timer logic: increments counttime if starttimer is active
        if (starttimer == 1) {
            counttime++;
            // Check if the main timer has reached its final quantity
            if ((finalquantity - counttime) < 1) {
                counttime = 0;     // Reset the main timer counter
                longtimecounter++;
                if(longtimecounter >= 144){ // 30seg * 2= 1min, 2* 60min = 120 (1.2 * 120 = )
                    finalquantity = 0; // Mark final quantity as reached
                    starttimer = 0;    // Stop the main timer
                    // If a process was started, activate the final buzzer
                if (processstarted == 1) {
                    finalbuzzer = 1;      // Activate the final buzzer sequence
                    processstarted = 0;   // Reset process started flag after triggering buzzer
                }
                }

                
            }
        }

        // Logic for the final buzzer sequence
        if (finalbuzzer == 1) {
            finalbuzzercounter++;
            canstartblinking = 0; // Stop LED blinking during buzzer sound
            if (finalbuzzercounter <= 3000) { // Buzzer active for 3 seconds (3000ms)
                buzzeron = 1;
            } else {
                buzzeron = 0;                 // Turn buzzer off
                // Reset all flags and counters to prepare for a new cycle
                starttimer = 1;               // Re-enable timer for next sequence (if needed, or set to 0 if full reset)
                                              // NOTE: Setting starttimer=1 here will immediately start counting again.
                                              // Consider if you want it to wait for a new button press.
                                              // If you want it to wait, set starttimer=0 here.
                processbuttonclicks = 0;
                processstarted = 0;
                finalbuzzercounter = 0;
                finalbuzzer = 0;
            }
        }

        // LED blinking logic during button click processing
        if (ledtimer >= 200 && processbuttonclicks > 0 && canstartblinking == 1) {
            processbuttonclicks--; // Decrement the blinking count
            if (start == 1) {
                start = 0; // Toggle LED state
                buzzeron = 0;
            } else {
                start = 1;
                buzzeron= 1;
            }
            ledtimer = 0; // Reset LED timer
        } else if (processbuttonclicks <= 0 && canstartblinking == 1) {
            // Once blinking is done, set the final quantity based on button clicks
            if (timecontrol == 4) {
                finalquantity = 30000; // 30 seconds
            } else if (timecontrol == 3) {
                finalquantity = 22500; // 22.5 seconds
            } else if (timecontrol == 2) {
                finalquantity = 15000; // 15 seconds
            } else if (timecontrol == 1) {
                finalquantity = 7500; // 7.5 seconds
            } else {
                finalquantity = 0; // Default to 2 seconds
            }
            // After setting final quantity, if not already started, start the main timer
            if (starttimer == 0 && finalbuzzer == 0) { // Ensure buzzer isn't active
                starttimer = 1; // Start the main timer countdown
                canstartblinking = 0; // Stop blinking after time is set
            }
        }

        // Control LED and Buzzer output based on state
        if ((start == 1 && starttimer == 0) || (starttimer == 1 && finalquantity != 0) || buzzeron == 1) {
            // LED is ON if blinking (start==1 and starttimer==0), or main timer is running, or buzzer is on
            LED = 1;
            // Buzzer is ON only if it's the final buzzer sequence
            if (buzzeron == 1) {
                BUZZER = 1;
            } else {
                BUZZER = 0; // Ensure buzzer is off if not in final buzzer state
            }
        } else {
            // LED is OFF if not blinking, main timer is finished, or buzzer is off
            LED = 0;
            BUZZER = 0; // Ensure buzzer is off
        }

        T0IF = 0; // Clear Timer0 interrupt flag
        TMR0 = 6; // Reload Timer0 to achieve 1ms interrupt (256 - 250 = 6)
    }
}

//////////////////////////////////////////////////////Main Routine///////////////////////////////////////////////////////////////
void main(void) {
    // Configure Comparator Module: Disable comparators (all pins are digital I/O)
    CMCON = 0x07;

    // Configure Analog-to-Digital Converter (ADC)
    // ANSEL: Analog Select Register
    // Bit 7-6: ADCS1:ADCS0 = 00 (Fosc/2) - Changed to 01 (Fosc/8) for better ADC performance
    // Bit 3: ANS3 = 0 (GP4/AN3 is digital I/O)
    // Bit 2: ANS2 = 0 (GP2/AN2 is digital I/O)
    // Bit 1: ANS1 = 0 (GP1/AN1 is digital I/O)
    // Bit 0: ANS0 = 1 (GP0/AN0 is analog input)
    ANSEL = 0b00100001; // Fosc/8 (ADCS1:ADCS0 = 01), AN0 as analog input

    // ADCON0: ADC Control Register 0
    // Bit 7: ADCS1 (part of ADC clock select)
    // Bit 6: ADCS0 (part of ADC clock select)
    // Bit 5-2: CHS1:CHS0 = 0000 (Channel 0 / AN0 selected)
    // Bit 1: GO_nDONE = 0 (A/D conversion status bit, cleared)
    // Bit 0: ADON = 1 (A/D converter module is enabled)
    ADCON0 = 0b10000001; // ADC enabled, select channel 0 (AN0/GP0), GO/DONE cleared

    // WPU: Weak Pull-Up Register (all disabled)
    WPU = 0X00;

    // TMR0: Timer0 Register (initial value)
    TMR0 = 0;

    // OSCCAL: Oscillator Calibration Register (set to max frequency for 4MHz)
    OSCCAL = 0XFF;

    // OPTION_REG: Option Register
    // Bit 7: RBPU = 1 (PORTB pull-ups disabled) - Not relevant for PIC12F675
    // Bit 6: INTEDG = 0 (Interrupt on falling edge of INT pin)
    // Bit 5: T0CS = 0 (Timer0 clock source is internal instruction cycle clock (Fosc/4))
    // Bit 4: T0SE = 0 (Timer0 source edge select bit)
    // Bit 3: PSA = 0 (Prescaler is assigned to Timer0)
    // Bit 2-0: PS2:PS0 = 001 (Prescaler 1:4 for Timer0)
    // Fosc = 4MHz -> Fosc/4 = 1MHz instruction clock.
    // With prescaler 1:4, Timer0 increments at 250kHz.
    OPTION_REG = 0X81; // Pull-ups disabled / Prescaler 1:4 for Timer0

    // INTCON: Interrupt Control Register
    // Bit 7: GIE = 1 (Global Interrupt Enable)
    // Bit 6: PEIE = 1 (Peripheral Interrupt Enable)
    // Bit 5: T0IE = 1 (Timer0 Overflow Interrupt Enable)
    // Bit 4: INTE = 0 (GP2/INT External Interrupt Enable)
    // Bit 3: GPIE = 0 (GP Interrupt Enable)
    // Bit 2: T0IF = 0 (Timer0 Overflow Interrupt Flag, cleared by hardware)
    // Bit 1: INTF = 0 (GP2/INT External Interrupt Flag)
    // Bit 0: GPIF = 0 (GP Interrupt Flag)
    INTCON = 0XE0; // Enable GIE, PEIE, T0IE

    // TRISIO: I/O Direction Register
    // Bit 5: GP5 (LED) = 0 (Output)
    // Bit 4: GP4 = 0 (Output)
    // Bit 3: GP3 = 0 (Output)
    // Bit 2: GP2 (BUZZER) = 0 (Output)
    // Bit 1: GP1 = 1 (Input)
    // Bit 0: GP0 (ANALOG_PIN) = 1 (Input)
    TRISIO = 0X03; // Configure GP0 and GP1 as input, others as output

    // Main program loop
    for (;;) {
        // Debounce and read button input every ~300ms (300 counts of 1ms timer)
        if (buttonstimer >= 300) {
            buttonstimer = 0; // Reset button timer

            adc_value = Read_Adc(); // Read ADC analog value from GP0

            // Check for button press (ADC value between 90 and 1023)
            // and ensure no process is currently blinking/timing
            if (adc_value > 90 && adc_value <= 1023 && canstartblinking == 0 && starttimer == 0) {
                buttonclicks++; // Increment button click counter
                processstarted = 1; // Set flag indicating a process has been initiated

                if (buttonclicks == 1) {
                    enterbuttontimercounter = 1; // Start the 4.6s window for multiple clicks
                } else if (buttonclicks > 4) {
                    buttonclicks = 4; // Limit clicks to a maximum of 4
                }
            }

            // Logic to process button clicks after a delay (4.6 seconds)
            if (enterbuttontimercounter == 1) {
                buttontimercounter++; // Increment counter for the 4.6s window
                if (buttontimercounter > 15) { // If 4.6 seconds (15 * 300ms approx) have passed
                    enterbuttontimercounter = 0; // Reset flag
                    buttontimercounter = 0;      // Reset counter
                    processbuttonclicks = 2 * buttonclicks; // Set LED blinking count (2 blinks per click)
                    timecontrol = buttonclicks;  // Store button clicks for time calculation
                    buttonclicks = 0;            // Reset button clicks for next sequence
                    canstartblinking = 1;        // Allow LED to start blinking
                }
            }
        }
        if(adc_value <= 90 && adc_value > 20){ // Stop button
            starttimer= 0;
            canstartblinking= 0;
            buttontimercounter= 0;
            processbuttonclicks= 0;
            buttonclicks= 0;
            processbuttonclicks= 0;
            finalbuzzer= 0;
        } 
    }
}
