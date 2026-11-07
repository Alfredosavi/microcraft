#include "bsp_led.h"

#include <msp430g2553.h>

#define LED_BIT BIT0


void led_init(void)
{
    P1OUT &= ~LED_BIT;
    P1SEL &= ~LED_BIT;
    P1SEL2 &= ~LED_BIT;
    P1DIR |= LED_BIT;
}


void led_on(void)
{
    P1OUT |= LED_BIT;
}


void led_off(void)
{
    P1OUT &= ~LED_BIT;
}


void led_toggle(void)
{
    P1OUT ^= LED_BIT;
}
