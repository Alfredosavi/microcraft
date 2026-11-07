#ifndef SPI_BSP_H
#define SPI_BSP_H

#include <stdbool.h>
#include <stdint.h>

void spi_init(void);
bool spi_transfer(uint8_t tx, uint8_t *rx);
void spi_cs_low(void);
void spi_cs_high(void);

#endif
