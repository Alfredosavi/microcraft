#include "bsp_delay.h"

#include "w5500_msp_config.h"


void delay_ms(uint16_t ms)
{
    while (ms--) {
        __delay_cycles(W5500_MSP_MCLK_HZ / 1000UL);
    }
}
