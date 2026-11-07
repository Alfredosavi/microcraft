#include <msp430.h>

#include <app_main.h>



void main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW + WDTHOLD;

    // Initialize the application
    app_main_run();

    // Should never reach here
    while (1) {
        __no_operation();
    }   
}
