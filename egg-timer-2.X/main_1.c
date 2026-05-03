// reference: https://microcontroladores-c.blogspot.com/2014/09/como-usar-o-timer-e-o-comparador-de.html
// this made the interrupt work: https://stackoverflow.com/questions/73883808/pic16f877a-timer1-interrupt-time-is-not-as-expected
// some ideas aboout analog here: https://forum.microchip.com/s/topic/a5CV40000001m0zMAA/t397074

// ADC values for buttons measured in 05/03/2026:
// Start button: 2,64V, Stop button: 1,69V

// FIX: _XTAL_FREQ must be before xc.h for __delay_ms to compile correctly
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

// ---------------- GLOBALS ----------------

volatile uint16_t ms_ticks = 0;

volatile uint8_t flag_20ms = 0;
volatile uint8_t flag_200ms = 0;

volatile uint8_t tick_20ms = 0;
volatile uint8_t tick_200ms = 0;

volatile uint8_t start = 0;
volatile uint16_t counttime = 0;
volatile uint8_t starttimer = 0;

volatile uint8_t processstarted = 0;
volatile uint8_t canstartblinking = 0;

volatile uint8_t processbuttonclicks = 0;
volatile uint8_t buttonclicks = 0;

volatile uint8_t timecontrol = 0;
volatile uint16_t finalquantity = 2000;

volatile uint8_t finalbuzzer = 0;
volatile uint16_t finalbuzzercounter = 0;
volatile uint8_t buzzeron = 0;

volatile uint16_t longtimecounter = 0;

volatile uint8_t click_window_active = 0;
volatile uint16_t click_window_timer = 0;

// ADC
uint16_t adc_value = 0;
uint16_t adc_prev1 = 0;
uint16_t adc_prev2 = 0;

// debounce
uint8_t last_button = 0;
uint8_t stable_button = 0;
uint8_t debounce_cnt = 0;

// Replace threshold block:
// 10-bit ADC thresholds for VDD=5V
// SW1: 5V * 47k/(68k+47k) = 2.04V -> (2.04/5.0)*1023 = ~418
// SW2: 5V * 47k/(47k+47k) = 2.50V -> (2.50/5.0)*1023 = ~511
#define BTN_NOISE_FLOOR  100
#define BTN_STOP_MIN     350
#define BTN_STOP_MAX     480
#define BTN_START_MIN    450
#define BTN_START_MAX    570

// ---------------- ADC ----------------

// Replace Read_Adc() entirely:
uint16_t Read_Adc(void) {
    // 10-bit result: ADRESH holds bits 9:8, ADRESL holds bits 7:0 (right-justified)
    // https://ww1.microchip.com/downloads/en/DeviceDoc/41190G.pdf section 6.1
    __delay_us(20);
    ADCON0bits.GO_nDONE = 1;
    while (ADCON0bits.GO_nDONE);
    return ((uint16_t)ADRESH << 8) | ADRESL;
}
// Replace get_button():
uint8_t get_button(uint16_t adc) {
    if (adc < BTN_NOISE_FLOOR)                       return 0; // floating/GND
    if (adc > BTN_START_MIN && adc < BTN_START_MAX)  return 2;
    if (adc > BTN_STOP_MIN  && adc < BTN_STOP_MAX)   return 1;
    return 0;
}

// ---------------- ISR ----------------

void __interrupt() ISR(void) {

    if (T0IF) {

        T0IF = 0;
        TMR0 = 6; // reload for 1ms tick

        ms_ticks++;

        // --- 20ms tick ---
        tick_20ms++;
        if (tick_20ms >= 20) {
            tick_20ms = 0;
            flag_20ms = 1;
        }

        // --- 200ms tick ---
        tick_200ms++;
        if (tick_200ms >= 200) {
            tick_200ms = 0;
            flag_200ms = 1;
        }

        // --- main timer ---
        if (starttimer) {
            counttime++;

            if (counttime >= finalquantity) {
                counttime = 0;
                longtimecounter++;

                if (longtimecounter >= 144) {
                    starttimer = 0;

                    if (processstarted) {
                        finalbuzzer = 1;
                        processstarted = 0;
                    }
                }
            }
        }

        // --- end buzzer ---
        if (finalbuzzer) {
            finalbuzzercounter++;

            if (finalbuzzercounter <= 3000) {
                buzzeron = 1;
            } else {
                buzzeron = 0;
                while (1); // halt system after buzzer
            }
        }

        // --- LED / buzzer output ---
        if ((start && !starttimer) ||
            (starttimer && finalquantity != 0) ||
            buzzeron) {
            LED    = 1;
            BUZZER = buzzeron;
        } else {
            LED    = 0;
            BUZZER = 0;
        }
    }
}

// ---------------- MAIN ----------------

void main(void) {

    // 1. Oscillator first ? factory cal value was lost, 0x80 is center value for rev B
    // https://ww1.microchip.com/downloads/en/DeviceDoc/41190G.pdf section 9.2
    OSCCAL = 0x80;
    uint16_t j;
    for (j = 0; j < 10000; j++) { CLRWDT(); } // let oscillator stabilize

    // 2. Disable comparator before using analog pins
    // https://ww1.microchip.com/downloads/en/DeviceDoc/41190G.pdf section 5.0
    CMCON = 0x07;

    // 3. Pin directions
    TRISIO = 0b00000001; // GP0 = input (ADC), all others = output

    // 4. Analog setup ? AN0 only, right-justified
    // https://ww1.microchip.com/downloads/en/DeviceDoc/41190G.pdf section 6.0
    ANSEL  = 0b00000001;
    ADCON0 = 0b00000001; // right-justify (bit7=0), AN0, ADC on
    __delay_ms(5); // acquisition time after ADON

    // 5. Timer0: internal clock, prescaler 1:4 -> 1ms overflow at 4MHz
    // https://ww1.microchip.com/downloads/en/DeviceDoc/41190G.pdf section 7.0
    OPTION_REG = 0x81;
    TMR0       = 0;
    TMR0IF     = 0;
    TMR0IE     = 1;
    INTCON = 0XE0; // Enable GIE, PEIE, T0IE

    while (1) {

        CLRWDT();

        // =============================
        // 20ms TASKS (ADC + BUTTON)
        // =============================
        if (flag_20ms) {
            flag_20ms = 0;

            // median filter over 3 samples
            adc_prev2 = adc_prev1;
            adc_prev1 = adc_value;
            uint16_t raw = Read_Adc();

            uint16_t a = raw;
            uint16_t b = adc_prev1;
            uint16_t c = adc_prev2;

            if      ((a >= b && a <= c) || (a >= c && a <= b)) adc_value = a;
            else if ((b >= a && b <= c) || (b >= c && b <= a)) adc_value = b;
            else                                                adc_value = c;

            // --- debounce ---
            uint8_t current = get_button(adc_value);

            if (current == last_button) {
                if (debounce_cnt < 3) debounce_cnt++;
            } else {
                debounce_cnt = 0;
            }

            if (debounce_cnt >= 3) {

                if (current != stable_button) {
                    stable_button = current;

                    // START button
                    if (stable_button == 2) {
                        if (!starttimer && !canstartblinking) {
                            buttonclicks++;
                            processstarted = 1;

                            if (buttonclicks == 1) {
                                click_window_active = 1;
                                click_window_timer  = 0;
                            }

                            if (buttonclicks > 4)
                                buttonclicks = 4;
                        }
                    }

                    // STOP button ? full reset
                    if (stable_button == 1) {
                        buttonclicks        = 0;
                        click_window_active = 0;
                        starttimer          = 0;
                        canstartblinking    = 0;
                        counttime           = 0;
                        longtimecounter     = 0;
                    }
                }
            }

            last_button = current;

            // --- multi-click window: 20ms * 200 = 4 seconds ---
            if (click_window_active) {
                click_window_timer++;

                if (click_window_timer >= 200) {
                    click_window_active  = 0;
                    processbuttonclicks  = buttonclicks * 2;
                    timecontrol          = buttonclicks;
                    buttonclicks         = 0;
                    canstartblinking     = 1;
                }
            }
        }

        // =============================
        // 200ms TASKS (LED feedback + timer start)
        // =============================
        if (flag_200ms) {
            flag_200ms = 0;

            if (processbuttonclicks > 0 && canstartblinking) {
                processbuttonclicks--;
                start   ^= 1;
                buzzeron = start;

            } else if (canstartblinking) {
                // 1 press=15min, 2=30min, 3=45min, 4=60min
                if      (timecontrol == 1) finalquantity = 7500;
                else if (timecontrol == 2) finalquantity = 15000;
                else if (timecontrol == 3) finalquantity = 22500;
                else if (timecontrol == 4) finalquantity = 30000;

                starttimer       = 1;
                canstartblinking = 0;
            }
        }
    }
}