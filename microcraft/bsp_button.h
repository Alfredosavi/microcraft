#ifndef BSP_BUTTON_H
#define BSP_BUTTON_H

#include <stdbool.h>

/* LaunchPad S2, active-low on P1.3. */

void button_init(void);
bool button_poll(void); // One event per debounced press. Call even when offline

#endif
