#ifndef BSP_LED_H
#define BSP_LED_H

// LED red P1.0. P1.6 is reserved for MISO.
void led_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);

#endif
