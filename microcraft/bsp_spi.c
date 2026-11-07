#include "bsp_spi.h"

#include "w5500_msp_config.h"
#include <stddef.h>

#define SPI_TIMEOUT_LOOPS 60000u



void spi_init(void)
{
    P1OUT |= SPI_CS_BIT;
    P1SEL &= ~SPI_CS_BIT;
    P1SEL2 &= ~SPI_CS_BIT;
    P1DIR |= SPI_CS_BIT;

    UCB0CTL1 |= UCSWRST;
    P1SEL |= BIT5 | BIT6 | BIT7;
    P1SEL2 |= BIT5 | BIT6 | BIT7;
    UCB0CTL0 = UCCKPH | UCMSB | UCMST | UCSYNC; /* Mode 0 USCI. */
    UCB0CTL1 = UCSSEL_2 | UCSWRST;
    UCB0BR0 = 4; /* SMCLK/4. */
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;
}


bool spi_transfer(uint8_t tx, uint8_t *rx)
{
    uint16_t timeout = SPI_TIMEOUT_LOOPS;

    if (rx == NULL) return false;
    
    while (!(IFG2 & UCB0TXIFG)) {
        if (--timeout == 0) return false;
    }

    if (IFG2 & UCB0RXIFG) (void)UCB0RXBUF;
    UCB0TXBUF = tx;
    timeout = SPI_TIMEOUT_LOOPS;

    while (!(IFG2 & UCB0RXIFG)) {
        if (--timeout == 0) return false;
    }

    *rx = UCB0RXBUF;
    return true;
}


void spi_cs_low(void) 
{ 
    P1OUT &= ~SPI_CS_BIT;
}


void spi_cs_high(void)
{
    P1OUT |= SPI_CS_BIT;
}
