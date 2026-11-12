#include "bsp_delay.h"

#include "w5500_msp_config.h"

static volatile uint16_t ticks;


void clock_init(void)
{
    /* SMCLK uses the same 1 MHz clock assumed by delay_ms and SPI. */
    TA0CTL = MC_0;
    ticks = 0;
    TA0CCR0 = (W5500_MSP_MCLK_HZ / 8UL / 1000UL) * BSP_CLOCK_TICK_MS - 1u;
    TA0CCTL0 = CCIE;
    TA0CTL = TASSEL_2 | ID_3 | MC_1 | TACLR;
    __enable_interrupt();
}


uint16_t clock_ticks(void)
{
    return ticks; /* Aligned 16-bit reads are atomic on MSP430. */
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void clock_tick_isr(void)
{
    ++ticks;
}


void delay_ms(uint16_t ms)
{
    while (ms--) {
        __delay_cycles(W5500_MSP_MCLK_HZ / 1000UL);
    }
}
