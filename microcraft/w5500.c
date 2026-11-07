#include "w5500.h"

#include "w5500_registers.h"
#include "bsp_spi.h"
#include "bsp_w5500.h"
#include "bsp_delay.h"

#include <stddef.h>



#define COMMAND_WAIT_MS 100u
#define SEND_WAIT_MS 5000u



static int16_t w5500_frame(uint16_t addr, uint8_t block, const uint8_t *tx,
                           uint8_t *rx, uint16_t len)
{
    uint8_t dummy;
    bool ret;

    spi_cs_low();
    ret = spi_transfer((uint8_t)(addr >> 8), &dummy) &&
              spi_transfer((uint8_t)addr, &dummy) &&
              spi_transfer((uint8_t)((block << 3) | (tx ? 4u : 0u)), &dummy);

    while (ret && len > 0) {
        ret = spi_transfer(tx ? *tx++ : 0, &dummy);

        if (ret && rx) *rx++ = dummy;
        --len;
    }

    spi_cs_high();

    return ret ? W5500_OK : W5500_ERR_IO;
}


static int16_t w5500_read8(uint16_t addr, uint8_t block)
{
    uint8_t value;
    int16_t rc = w5500_frame(addr, block, NULL, &value, 1);
    return rc < 0 ? rc : (int16_t)value;
}


static int16_t w5500_write8(uint16_t addr, uint8_t block, uint8_t value)
{
    return w5500_frame(addr, block, &value, NULL, 1);
}


static int16_t w5500_read16(uint16_t addr, uint16_t *value)
{
    uint8_t bytes[2];
    int16_t rc = w5500_frame(addr, BSB_S0_REG, NULL, bytes, 2);
    if (rc == 0) *value = (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
    return rc;
}


static int16_t w5500_write16(uint16_t addr, uint16_t value)
{
    uint8_t bytes[2] = {(uint8_t)(value >> 8), (uint8_t)value};
    return w5500_frame(addr, BSB_S0_REG, bytes, NULL, 2);
}


static int16_t w5500_read_size(uint16_t addr)
{
    uint16_t previous, current;
    uint8_t tries;

    int16_t rc = w5500_read16(addr, &previous);
    if (rc < 0) return rc;

    for (tries = 0; tries < 16; ++tries) {
        rc = w5500_read16(addr, &current);
        if (rc < 0) return rc;

        if (current == previous)
            return current <= 16384u ? (int16_t)current : W5500_ERR_IO;
        previous = current;
    }

    return W5500_ERR_TIMEOUT;
}


int16_t w5500_link_up(void)
{
    int16_t rc = w5500_read8(W5500_COMMON_PHYCFGR, BSB_COMMON_REG);
    return rc < 0 ? rc : (rc & 1);
}


int16_t w5500_init(const w5500_network_t *network)
{
    uint8_t socket;

    // Verify MAC address is unicast (LSB of first byte must be 0).
    if (!network || (network->mac[0] & 1u)) return W5500_ERR_ARG;

    // Initialize SPI and reset the W5500 chip
    spi_init();
    w5500_reset_pin_init();
    spi_cs_high();
    w5500_reset();

    int16_t rc = w5500_read8(W5500_COMMON_VERSIONR, BSB_COMMON_REG);

    if (rc < 0) return rc;
    if (rc != 4) return W5500_ERR_VERSION;

    for (socket = 0; socket < 8; ++socket) {
        uint8_t block = (uint8_t)(1u + 4u * socket);

        rc = w5500_write8(SN_RXBUF_SIZE, block, 0);
        if (rc < 0) return rc;
        
        rc = w5500_write8(SN_TXBUF_SIZE, block, 0);
        if (rc < 0) return rc;
    }


    rc = w5500_write8(SN_RXBUF_SIZE, BSB_S0_REG, 16);
    if (rc < 0) return rc;

    rc = w5500_write8(SN_TXBUF_SIZE, BSB_S0_REG, 16);
    if (rc < 0) return rc;

    rc = w5500_frame(W5500_COMMON_SHAR0, BSB_COMMON_REG, network->mac, NULL, 6);
    if (rc < 0) return rc;

    rc = w5500_frame(W5500_COMMON_SIPR0, BSB_COMMON_REG, network->ip, NULL, 4);
    if (rc < 0) return rc;

    rc = w5500_frame(W5500_COMMON_SUBR0, BSB_COMMON_REG, network->subnet, NULL, 4);
    if (rc < 0) return rc;
    
    return w5500_frame(W5500_COMMON_GAR0, BSB_COMMON_REG, network->gateway, NULL, 4);
}


static int16_t socket_command(uint8_t cmd)
{
    uint16_t remaining = COMMAND_WAIT_MS;
    int16_t rc = w5500_write8(SN_CR, BSB_S0_REG, cmd);

    if (rc < 0) return rc;

    do {
        rc = w5500_read8(SN_CR, BSB_S0_REG);
        if (rc <= 0) return rc;
        delay_ms(1);
    } while (--remaining);

    return W5500_ERR_TIMEOUT;
}


int16_t w5500_socket_status(void)
{
    return w5500_read8(SN_SR, BSB_S0_REG);
}


int16_t w5500_tcp_close(void)
{
    int16_t rc = socket_command(W5500_CMD_CLOSE);
    if (rc < 0) return rc;
    return w5500_write8(SN_IR, BSB_S0_REG, 0x1f);
}


static int16_t socket_abort(int16_t error)
{
    (void)w5500_tcp_close();
    return error;
}


int16_t w5500_tcp_listen(uint16_t port)
{
    int16_t rc;

    if (!port) return W5500_ERR_ARG;

    rc = w5500_tcp_close();
    if (rc < 0) return rc;

    rc = w5500_write8(SN_MR, BSB_S0_REG, 1);
    if (rc < 0) return rc;

    rc = w5500_write16(SN_PORT0, port);
    if (rc < 0) return rc;

    rc = socket_command(W5500_CMD_OPEN);
    if (rc < 0) return socket_abort(rc);

    rc = w5500_socket_status();
    if (rc != W5500_SOCK_INIT)
        return socket_abort(rc < 0 ? rc : W5500_ERR_STATE);

    rc = socket_command(W5500_CMD_LISTEN);
    if (rc < 0) return socket_abort(rc);

    rc = w5500_socket_status();
    if (rc == W5500_SOCK_LISTEN || rc == W5500_SOCK_ESTABLISHED ||
        rc == W5500_SOCK_CLOSE_WAIT || rc == 0x15 /* SYNRECV */)
        return W5500_OK;

    return socket_abort(rc < 0 ? rc : W5500_ERR_STATE);
}


int16_t w5500_tcp_disconnect(void)
{
    int16_t rc = w5500_socket_status();
    if (rc < 0) return rc;

    if (rc != W5500_SOCK_ESTABLISHED && rc != W5500_SOCK_CLOSE_WAIT)
        return W5500_ERR_STATE;

    rc = socket_command(W5500_CMD_DISCON);
    return rc < 0 ? socket_abort(rc) : rc;
}


int16_t w5500_rx_available(void)
{
    return w5500_read_size(SN_RX_RSR0);
}


int16_t w5500_tcp_recv(uint8_t *buffer, uint16_t capacity)
{
    uint16_t pointer, len;
    int16_t rc;

    if (!capacity) return 0;
    if (!buffer) return W5500_ERR_ARG;

    rc = w5500_socket_status();
    if (rc < 0) return rc;

    if (rc != W5500_SOCK_ESTABLISHED && rc != W5500_SOCK_CLOSE_WAIT)
        return W5500_ERR_STATE;

    rc = w5500_rx_available();
    if (rc <= 0) return rc;

    len = (uint16_t)rc < capacity ? (uint16_t)rc : capacity;
    rc = w5500_read16(SN_RX_RD0, &pointer);
    if (rc < 0) return rc;

    // The chip applies the buffer wrap; pointers remain 16 bits.
    rc = w5500_frame(pointer, BSB_S0_RX_BUF, NULL, buffer, len);
    if (rc < 0) return rc;

    rc = w5500_write16(SN_RX_RD0, (uint16_t)(pointer + len));
    if (rc < 0) return socket_abort(rc);

    rc = socket_command(W5500_CMD_RECV);
    return rc < 0 ? socket_abort(rc) : (int16_t)len;
}


int16_t w5500_tcp_send(const uint8_t *data, uint16_t len)
{
    uint16_t pointer, remaining = SEND_WAIT_MS;
    int16_t rc;

    if (!len) return 0;
    if (!data) return W5500_ERR_ARG;
    
    rc = w5500_socket_status();
    if (rc < 0) return rc;
    
    if (rc != W5500_SOCK_ESTABLISHED && rc != W5500_SOCK_CLOSE_WAIT)
        return W5500_ERR_STATE;

    rc = w5500_read_size(SN_TX_FSR0);
    if (rc <= 0) return rc;

    if (len > (uint16_t)rc) len = (uint16_t)rc;
    rc = w5500_read16(SN_TX_WR0, &pointer);
    if (rc < 0) return rc;

    rc = w5500_frame(pointer, BSB_S0_TX_BUF, data, NULL, len);
    if (rc < 0) return socket_abort(rc);

    rc = w5500_write8(SN_IR, BSB_S0_REG, W5500_IR_SEND_OK | W5500_IR_TIMEOUT);
    if (rc < 0) return socket_abort(rc);

    rc = w5500_write16(SN_TX_WR0, (uint16_t)(pointer + len));
    if (rc < 0) return socket_abort(rc);

    rc = socket_command(W5500_CMD_SEND);
    if (rc < 0) return socket_abort(rc);


    do {
        rc = w5500_read8(SN_IR, BSB_S0_REG);

        if (rc < 0) return socket_abort(rc);
        if (rc & W5500_IR_TIMEOUT) return socket_abort(W5500_ERR_TIMEOUT);

        if (rc & W5500_IR_SEND_OK) {
            rc = w5500_write8(SN_IR, BSB_S0_REG, W5500_IR_SEND_OK);
            return rc < 0 ? socket_abort(rc) : (int16_t)len;
        }


        rc = w5500_socket_status();

        if (rc < 0) return socket_abort(rc);
        if (rc != W5500_SOCK_ESTABLISHED && rc != W5500_SOCK_CLOSE_WAIT)
            return socket_abort(W5500_ERR_STATE);

        delay_ms(1);
    } while (--remaining);

    
    return socket_abort(W5500_ERR_TIMEOUT);
}
