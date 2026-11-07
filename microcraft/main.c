#include <msp430.h>

#include <stdbool.h>


#define LED_BIT BIT0



void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;

    // LED BLINK
    P1OUT &= ~LED_BIT;
    P1SEL &= ~LED_BIT;
    P1SEL2 &= ~LED_BIT;
    P1DIR |= LED_BIT;

    P1DIR |= BIT0;

    while (true) {
        P1OUT ^= LED_BIT;
        __delay_cycles(100000);
    }
}
