#include "app_main.h"

#include "app_config.h"
#include "mc_server.h"

#include "bsp_delay.h"
#include "bsp_led.h"
#include "bsp_button.h"
#include "w5500.h"

static const w5500_network_t network = {
    APP_MAC_ADDRESS,
    APP_IP_ADDRESS,
    APP_SUBNET_MASK,
    APP_GATEWAY_ADDRESS
};


static uint16_t closing_ms;



int16_t app_main_init(void);
static int16_t app_error(int16_t error);
int16_t app_main_poll(void);


// MAIN APP
void app_main_run(void)
{
    clock_init();
    
    // MAIN LOOP
    while (true) {
        if (app_main_init() == W5500_OK) {
            while (app_main_poll() == W5500_OK) {
                delay_ms(APP_POLL_INTERVAL_MS);
            }
        }

        delay_ms(APP_RETRY_DELAY_MS);
    }
}


int16_t app_main_init(void)
{
    mc_server_reset();
    closing_ms = 0;
    led_init();
    button_init();

    return w5500_init(&network);
}


static int16_t app_error(int16_t error)
{
    int16_t rc = w5500_tcp_close();

    mc_server_reset();
    closing_ms = 0;
    if (rc < 0) return rc;
    
    // Protocol errors and Keep Alive timeouts only close the current connection.
    if (error == MC_ERR_PACKET || error == MC_ERR_VERSION || error == MC_ERR_TIMEOUT)
        return W5500_OK;
    return error;
}


int16_t app_main_poll(void)
{
    bool pressed = button_poll();
    int16_t rc = w5500_socket_status();
    if (rc < 0) return app_error(rc);

    if (rc == W5500_SOCK_CLOSED) {
        mc_server_reset();
        closing_ms = 0;
        rc = w5500_link_up();
        if (rc <= 0) return rc;

        return w5500_tcp_listen(APP_TCP_PORT);
    }

    if (rc != W5500_SOCK_ESTABLISHED && rc != W5500_SOCK_CLOSE_WAIT) {
        // FIN_WAIT/LAST_ACK: limit to accept new connections again.
        if (closing_ms) {
            delay_ms(1);
            if (--closing_ms == 0) return w5500_tcp_close();
        }

        return W5500_OK;
    }

    if (rc == W5500_SOCK_ESTABLISHED && pressed) {
        rc = mc_server_toggle_devices();
        if (rc < 0) return app_error(rc);
    }

    rc = mc_server_poll();
    if (rc < 0) return app_error(rc);

    rc = w5500_socket_status();
    if (rc < 0) return app_error(rc);

    if (rc == W5500_SOCK_CLOSE_WAIT) {
        rc = w5500_rx_available();
        if (rc < 0) return app_error(rc);
        if (rc == 0) {
            if (!mc_server_at_boundary()) return app_error(MC_ERR_PACKET);
            closing_ms = APP_CLOSE_WAIT_MS;
            return w5500_tcp_disconnect();
        }
    }

    return W5500_OK;
}
