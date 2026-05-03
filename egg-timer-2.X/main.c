// reference: https://microcontroladores-c.blogspot.com/2014/09/como-usar-o-timer-e-o-comparador-de.html
// this made the interrupt work: https://stackoverflow.com/questions/73883808/pic16f877a-timer1-interrupt-time-is-not-as-expected
// some ideas aboout analog here: https://forum.microchip.com/s/topic/a5CV40000001m0zMAA/t397074

// ADC values for buttons measured in 05/03/2026:
// Start button: 2,64V, Stop button: 1,69V
// For 8bit that means Start= 134, Stop= 86 (out of 255 or 2^8)

#define _XTAL_FREQ 4000000
#include <xc.h>
#include <stdint.h>

#define LED GP5
#define BUZZER GP2

#pragma config FOSC = INTRCIO
#pragma config WDTE = ON
#pragma config PWRTE = OFF
#pragma config MCLRE = OFF
#pragma config BOREN = OFF
#pragma config CP = OFF
#pragma config CPD = OFF

// Global variables for timer and state management

int buttonpressed = 0;
volatile int ledtimer = 0;
volatile int buttonstimer = 0;
// FIX: adtimer must be volatile ? modified in ISR, read in main()
volatile int adtimer = 0;
volatile unsigned char start = 0;
volatile unsigned char startbutton = 0;
volatile unsigned int adc_value = 0;
volatile unsigned char canstartblinking = 0;
volatile int processbuttonclicks = 0;
volatile int buttonclicks = 0;
int enterbuttontimercounter = 0;
volatile int buttontimercounter = 0;
volatile unsigned char starttimer = 0;
volatile int counttime = 0;
volatile unsigned char timecontrol = 0;
volatile int finalquantity = 2000;
volatile unsigned char finalbuzzer = 0;
volatile int finalbuzzercounter = 0;
volatile unsigned char buzzeron = 0;
volatile unsigned char processstarted = 0;
volatile int longtimecounter= 0;
volatile int dothemagicofreset= 0;
int thirdadc= 0;
int secondadc= 0;
int currentadc= 0;
int d1 = 0;
int d2 = 0;
int d3 = 0;
int last_adc_state = 0;

// Function to read ADC value from AN0 (GP0)
unsigned int Read_Adc(void) {
    ADCON0bits.GO_nDONE = 1;
    while (ADCON0bits.GO_nDONE);
    return ((unsigned int)ADRESH << 8) | ADRESL;
}

// Interrupt Service Routine (ISR)
void __interrupt() ISR() {
    if (T0IF) {
        
        ledtimer++;
        buttonstimer++;
        adtimer++;

        if (starttimer == 1) {
            counttime++;
            if ((finalquantity - counttime) < 1) {
                counttime = 0;
                longtimecounter++;
                if(longtimecounter >= 144){
                    finalquantity = 0;
                    starttimer = 0;
                    if (processstarted == 1) {
                        finalbuzzer = 1;
                        processstarted = 0;
                    }
                }
            }
        }

        if (finalbuzzer == 1) {
            finalbuzzercounter++;
            canstartblinking = 0;
            if (finalbuzzercounter <= 3000) {
                buzzeron = 1;
            } else {
                buzzeron = 0;
                processbuttonclicks = 0;
                processstarted = 0;
                finalbuzzercounter = 0;
                finalbuzzer = 0;
                while(1);
            }
        }

        if (ledtimer >= 200 && processbuttonclicks > 0 && canstartblinking == 1) {
            processbuttonclicks--;
            ledtimer = 0;
            // odd = ON, even = OFF ? guarantees sequence always ends on OFF
            if (processbuttonclicks % 2 == 0) {
                start    = 0;
                buzzeron = 0;
            } else {
                start    = 1;
                buzzeron = 1;
            }
        } else if (processbuttonclicks <= 0 && canstartblinking == 1) {
            // ensure outputs are off before starting timer
            start    = 0;
            buzzeron = 0;
            // total_ms = finalquantity * 144
            // 15min = 900,000ms -> 900,000/144 = 6,250
            // 30min = 1,800,000ms -> 1,800,000/144 = 12,500
            // 45min = 2,700,000ms -> 2,700,000/144 = 18,750
            // 60min = 3,600,000ms -> 3,600,000/144 = 25,000
            if      (timecontrol == 4) finalquantity = 25000;
            else if (timecontrol == 3) finalquantity = 18750;
            else if (timecontrol == 2) finalquantity = 12500;
            else if (timecontrol == 1) finalquantity = 6250;
            else                       finalquantity = 0;
            if (starttimer == 0 && finalbuzzer == 0) {
                starttimer       = 1;
                canstartblinking = 0;
            }
        }

        if ((start == 1 && starttimer == 0) || (starttimer == 1 && finalquantity != 0) || buzzeron == 1) {
            LED = 1;
            if (buzzeron == 1) {
                BUZZER = 1;
            } else {
                BUZZER = 0;
            }
        } else {
            LED = 0;
            BUZZER = 0;
        }

        T0IF = 0;
        TMR0 = 6;
    }
}

void main(void) {
    
    CMCON = 0x07;

    ANSEL = 0b00100001;
    ADCON0 = 0b10000001;

    WPU = 0X00;
    TMR0 = 0;

    // FIX: 0xFF is wrong for rev B ? factory cal value was lost
    // 0x80 is the center value for PIC12F675 revision B
    // https://ww1.microchip.com/downloads/en/DeviceDoc/41190G.pdf section 9.2
    OSCCAL = 0x80;

    OPTION_REG = 0X81;
    INTCON = 0XE0;
    TRISIO = 0X03;

    for (;;) {
        CLRWDT();
        
        if(adtimer >= 20){
            adtimer   = 0;
            thirdadc  = secondadc;
            secondadc = currentadc;
            currentadc = Read_Adc();

            // FIX: proper median of 3
            // https://en.wikipedia.org/wiki/Median_filter
            if ((currentadc >= secondadc && currentadc <= thirdadc) ||
                (currentadc >= thirdadc  && currentadc <= secondadc))
                adc_value = currentadc;
            else if ((secondadc >= currentadc && secondadc <= thirdadc) ||
                     (secondadc >= thirdadc   && secondadc <= currentadc))
                adc_value = secondadc;
            else
                adc_value = thirdadc;

            // --- button edge detection at 20ms resolution ---
            // only count on low->high transition, not while held
            int current_adc_state = (adc_value > 70) ? 1 : 0;
            // stop button range (adjust to your measured voltage)
            if (adc_value > 20 && adc_value <= 100) {
                dothemagicofreset = 1;
            }
            
            if (current_adc_state == 1 && last_adc_state == 0) {
                // start button range
                if (canstartblinking == 0 && starttimer == 0 && adc_value > 100) {
                    buttonclicks++;
                    processstarted = 1;

                    // only set window on first click, never reset it mid-sequence
                    if (buttonclicks == 1) {
                        enterbuttontimercounter = 1;
                        buttontimercounter      = 0;
                    }
                    if (buttonclicks > 4) buttonclicks = 4;
                }
            }
            last_adc_state = current_adc_state;
        }

        if (buttonstimer >= 300) {
            buttonstimer = 0;

            // --- multi-click window: 20 * 300ms = 6 seconds ---
            if (enterbuttontimercounter == 1) {
                buttontimercounter++;

                if (buttontimercounter > 20) {
                    enterbuttontimercounter = 0;
                    buttontimercounter      = 0;
                    processbuttonclicks     = 2 * buttonclicks;
                    timecontrol             = buttonclicks;
                    buttonclicks            = 0;
                    canstartblinking        = 1;
                }
            }
        }

        if(dothemagicofreset == 1) {
            // all we do here is lock the microcontroller in a while(1)
            // loop, so the watchdog quickly resets it.
            while(1);
        }    
    }
}
