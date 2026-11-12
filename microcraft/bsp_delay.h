#ifndef BSP_DELAY_H
#define BSP_DELAY_H

#include <stdint.h>

#define BSP_CLOCK_TICK_MS 10u

void clock_init(void);
uint16_t clock_ticks(void);
void delay_ms(uint16_t ms);

#endif
