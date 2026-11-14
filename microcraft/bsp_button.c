#include "bsp_button.h"

#include "bsp_delay.h"
#include <msp430g2553.h>

#define BUTTON_BIT BIT3
#define BUTTON_DEBOUNCE_MS 30u

static uint16_t changed_at;
static bool candidate;
static bool stable;


void button_init(void)
{
    P1SEL &= ~BUTTON_BIT;
    P1SEL2 &= ~BUTTON_BIT;
    P1DIR &= ~BUTTON_BIT;
    P1OUT |= BUTTON_BIT;
    P1REN |= BUTTON_BIT;
    P1IE &= ~BUTTON_BIT; // Polling

    candidate = stable = (P1IN & BUTTON_BIT) == 0;
    changed_at = clock_ticks();
}


bool button_poll(void)
{
    bool pressed = (P1IN & BUTTON_BIT) == 0;
    uint16_t now = clock_ticks();

    if (pressed != candidate) {
        candidate = pressed;
        changed_at = now;
    }

    if (candidate != stable &&
        (uint16_t)(now - changed_at) >= BUTTON_DEBOUNCE_MS / BSP_CLOCK_TICK_MS) {
        stable = candidate;
        return stable;
    }

    return false;
}
