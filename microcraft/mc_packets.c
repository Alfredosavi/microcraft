#include "mc_packets.h"

#include "mc_config.h"
#include "w5500.h"
#include "bsp_delay.h"



static const char status_json[] = MC_STATUS_JSON;

// ID Login Success + sizeof(UUID) + UUID. UUID=0 para singleplayer offline.
static const uint8_t login_identity[] = "\x02\x24" "00000000-0000-0000-0000-000000000000";
static const uint8_t pong_header[] = {9, 1};

 /* Join Game and Position And Look, including lengths, in big-endian.
  * Entity 1, survival, overworld, easy, default; position (0, 70, 0).
  * Y=70 in IEEE754: 0x4051800000000000; flags=0 and teleport ID=1. */
static const uint8_t play_start[] = {
    21, 0x23, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1,
    7, 'd', 'e', 'f', 'a', 'u', 'l', 't', 0,
    35, 0x2f,
    0, 0, 0, 0, 0, 0, 0, 0,
    0x40, 0x51, 0x80, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1
};


static int16_t send_all(const uint8_t *data, uint16_t len)
{
    uint16_t wait = MC_TX_WAIT_MS;

    while (len > 0) {
        int16_t rc = w5500_tcp_send(data, len);
        if (rc < 0) return rc;

        if (rc == 0) {
            if (wait == 0) return W5500_ERR_TIMEOUT;
            --wait;
            delay_ms(1);
        } else {
            data += (uint16_t)rc;
            len = (uint16_t)(len - (uint16_t)rc);
        }
    }

    return W5500_OK;
}


static uint8_t varint_size(uint32_t value)
{
    uint8_t len = 1;
    while (value >= 128u) { 
        value >>= 7; 
        ++len; 
    }

    return len;
}


static int16_t send_varint(uint32_t value)
{
    uint8_t bytes[5];
    uint8_t len = 0;

    do {
        uint8_t byte = (uint8_t)(value & 0x7fu);
        value >>= 7;
        if (value) byte |= 0x80u;
        bytes[len++] = byte;
    } while (value);

    return send_all(bytes, len);
}


int16_t mc_send_status(void)
{
    const uint16_t len = (uint16_t)(sizeof(status_json) - 1u);

    int16_t rc = send_varint(1u + varint_size(len) + len);
    if (rc < 0) return rc;

    rc = send_varint(0);
    if (rc < 0) return rc;

    rc = send_varint(len);
    return rc < 0 ? rc : send_all((const uint8_t *)status_json, len);
}


int16_t mc_send_pong(const uint8_t payload[8])
{
    int16_t rc = send_all(pong_header, sizeof(pong_header));
    return rc < 0 ? rc : send_all(payload, 8);
}


int16_t mc_send_login(const uint8_t *name, uint8_t length)
{
    int16_t rc = send_varint(1u + 1u + 36u + 1u + length);
    if (rc < 0) return rc;

    rc = send_all(login_identity, sizeof(login_identity) - 1u);
    if (rc < 0) return rc;

    rc = send_varint(length);
    if (rc < 0) return rc;
    
    rc = send_all(name, length);
    if (rc < 0) return rc;
    
    return send_all(play_start, sizeof(play_start));
}
