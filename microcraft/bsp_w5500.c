#include "bsp_w5500.h"

#include "bsp_delay.h"
#include "w5500_msp_config.h"



void w5500_reset_pin_init(void)
{
    P2OUT |= W5500_RST_BIT;
    P2SEL &= ~W5500_RST_BIT;
    P2SEL2 &= ~W5500_RST_BIT;
    P2DIR |= W5500_RST_BIT;
}



void w5500_reset(void)
{
    P2OUT &= ~W5500_RST_BIT;
    delay_ms(W5500_RESET_LOW_MS);
    P2OUT |= W5500_RST_BIT;
    delay_ms(W5500_RESET_RECOVERY_MS);
}
