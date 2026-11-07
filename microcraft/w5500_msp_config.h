#ifndef W5500_MSP_CONFIG_H
#define W5500_MSP_CONFIG_H

#include <msp430g2553.h>

/* SPI: P1.5=SCK, P1.6=MISO, P1.7=MOSI; CS=P1.4, RSTn=P2.0. */
#define SPI_CS_BIT BIT4
#define W5500_RST_BIT BIT0

// Do not change this value, it is used to calculate the SPI clock divider.
#define W5500_MSP_MCLK_HZ 1000000UL

#define W5500_RESET_LOW_MS 1u
#define W5500_RESET_RECOVERY_MS 100u

#endif
