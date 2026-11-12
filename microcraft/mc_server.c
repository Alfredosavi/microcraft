#include "mc_server.h"

#include "mc_config.h"
#include "mc_packets.h"

#include "w5500.h"
#include "bsp_led.h"
#include "bsp_delay.h"
#include <string.h>

enum { HANDSHAKE, STATUS, LOGIN, PLAY };
enum {
    LENGTH, ID, PROTOCOL, ADDRESS_LENGTH, ADDRESS, NEXT_STATE,
    NAME_LENGTH, NAME, PING, SKIP,
    HANDSHAKE_END, STATUS_END, PING_END, LOGIN_END,
    KEEPALIVE, KEEPALIVE_END, TELEPORT_ID, TELEPORT_END
};



static struct {
    uint32_t remaining;
    uint32_t varint; // Acumulator for VarInt; during ADDRESS counts address + port to discard.
    uint8_t state;
    uint8_t field;
    uint8_t varint_bytes;
    union {
        uint8_t used;          /* NAME/PING */
        bool version_matches; /* HANDSHAKE */
    } detail;

    uint8_t payload[MC_USERNAME_BYTES];
    uint16_t keepalive_at;
    uint16_t keepalive_id;
    bool keepalive_pending;
    bool teleport_pending;
} session;



void mc_server_reset(void)
{
    memset(&session, 0, sizeof(session)); /* HANDSHAKE, LENGTH */
}


bool mc_server_at_boundary(void)
{
    return session.field == LENGTH && session.varint_bytes == 0;
}


// 0=incomplete, 1=complete value, negative=invalid. Unsigned shifts limited to 28 bits; the fifth byte cannot exceed the 32 bits of the VarInt.
static int16_t read_varint_byte(uint8_t byte, uint32_t *value)
{
    if (session.varint_bytes == 4 && (byte & 0xf0u)) return MC_ERR_PACKET;

    session.varint |= (uint32_t)(byte & 0x7fu) << (7u * session.varint_bytes);
    ++session.varint_bytes;
    
    if (byte & 0x80u) return 0;
    
    *value = session.varint;
    session.varint = 0;
    session.varint_bytes = 0;
    
    return 1;
}


static int16_t start_packet(uint32_t id)
{
    session.detail.used = 0;

    switch (session.state) {
        case HANDSHAKE:
            session.field = id == 0 ? PROTOCOL : SKIP;
        break;

        case STATUS:
            if (id == 0) {
                if (session.remaining != 0) return MC_ERR_PACKET;
                session.field = STATUS_END;
            } else if (id == 1) {
                if (session.remaining != 8) return MC_ERR_PACKET;
                session.field = PING;
            } else {
                session.field = SKIP;
            }
        break;

        case LOGIN:
            session.field = id == 0 ? NAME_LENGTH : SKIP;
        break;

        case PLAY:
            if (id == 0x00) {
                if (!session.teleport_pending) return MC_ERR_PACKET;
                session.field = TELEPORT_ID;
            } else if (id == 0x0b) {
                if (session.remaining != 8 || !session.keepalive_pending)
                    return MC_ERR_PACKET;
                session.field = KEEPALIVE;
            } else {
                session.field = SKIP;
            }
        break;
    }

    return W5500_OK;
}


static int16_t finish_packet(void)
{
    int16_t rc = W5500_OK;

    switch (session.field) {
        case HANDSHAKE_END:
        break;

        case STATUS_END: 
            rc = mc_send_status();
        break;

        case PING_END: 
            rc = mc_send_pong(session.payload); 
        break;

        case LOGIN_END:
            rc = mc_send_login(session.payload, session.detail.used);
            if (rc == W5500_OK) {
                session.state = PLAY;
                session.keepalive_at = clock_ticks();
                session.teleport_pending = true;
            }
        break;

        case TELEPORT_END:
            session.teleport_pending = false;
        break;

        case KEEPALIVE_END:
            /* The ID uses only the low 16 bits, but validate all eight bytes. */
            if (session.payload[0] || session.payload[1] || session.payload[2] ||
                session.payload[3] || session.payload[4] || session.payload[5] ||
                session.payload[6] != (uint8_t)(session.keepalive_id >> 8) ||
                session.payload[7] != (uint8_t)session.keepalive_id)
                return MC_ERR_PACKET;
            session.keepalive_pending = false;
            session.keepalive_at = clock_ticks();
        break;

        case SKIP:
        break;
        
        default: 
            return MC_ERR_PACKET; // Packet truncated;
    }
    
    session.field = LENGTH;
    return rc;
}


static int16_t consume_field(uint8_t byte)
{
    uint32_t value;
    int16_t rc;

    switch (session.field) {
        case ADDRESS:
            if (--session.varint == 0) session.field = NEXT_STATE;
        return W5500_OK;

        case NAME:
            session.payload[session.detail.used++] = byte;
            if (session.remaining == 0) session.field = LOGIN_END;
        return W5500_OK;

        case PING:
            session.payload[session.detail.used++] = byte;
            if (session.remaining == 0) session.field = PING_END;
        return W5500_OK;

        case KEEPALIVE:
            session.payload[session.detail.used++] = byte;
            if (session.remaining == 0) session.field = KEEPALIVE_END;
        return W5500_OK;

        case SKIP:
            return W5500_OK;

        case HANDSHAKE_END:
        case STATUS_END:
        case PING_END:
        case LOGIN_END:
        case KEEPALIVE_END:
        case TELEPORT_END:
            return MC_ERR_PACKET;

        default:
        break;
    }

    rc = read_varint_byte(byte, &value);
    if (rc <= 0) return rc;

    switch (session.field) {
        case ID:
            if (value > 0x7fffffffu) return MC_ERR_PACKET;
            return start_packet(value);

        case PROTOCOL:
            session.detail.version_matches = value == MC_PROTOCOL_VERSION;
            session.field = ADDRESS_LENGTH;
        break;  

        case ADDRESS_LENGTH:
            if (value > 1020u || value + 3u > session.remaining) return MC_ERR_PACKET;

            session.varint = value + 2u; // Address and port
            session.field = ADDRESS;
        break;

        case NEXT_STATE:
            if (value != 1 && value != 2) return MC_ERR_PACKET;
            if (value == 2 && !session.detail.version_matches) return MC_ERR_VERSION;

            session.state = value == 1 ? STATUS : LOGIN;
            session.field = HANDSHAKE_END;
        break;

        case TELEPORT_ID:
            if (value != MC_INITIAL_TELEPORT_ID || session.remaining != 0)
                return MC_ERR_PACKET;
            session.field = TELEPORT_END;
        break;

        case NAME_LENGTH:
            if (value == 0 || value > MC_USERNAME_BYTES || value != session.remaining)
                return MC_ERR_PACKET;

            session.field = NAME;
        break;

        default:
            return MC_ERR_PACKET;
    }

    return W5500_OK;
}


static int16_t consume_byte(uint8_t byte)
{
    int16_t rc;
    if (session.field == LENGTH) {
        uint32_t length;
        
        if (session.varint_bytes == 2 && (byte & 0x80u)) return MC_ERR_PACKET;
        
        rc = read_varint_byte(byte, &length);
        if (rc <= 0) return rc;
        if (length == 0 || length > MC_MAX_PACKET_BYTES) return MC_ERR_PACKET;
        
        session.remaining = length;
        session.field = ID;
        return W5500_OK;
    }

    --session.remaining;
    rc = consume_field(byte);

    if (rc < 0) return rc;

    return session.remaining == 0 ? finish_packet() : W5500_OK;
}


static int16_t poll_keepalive(void)
{
    uint16_t now;
    uint16_t elapsed;
    int16_t rc;

    if (session.state != PLAY) return W5500_OK;

    now = clock_ticks();
    elapsed = (uint16_t)(now - session.keepalive_at); /* Handles tick wraparound. */
    if (session.keepalive_pending)
        return elapsed >= MC_KEEPALIVE_TIMEOUT_MS / BSP_CLOCK_TICK_MS
            ? MC_ERR_TIMEOUT : W5500_OK;

    if (elapsed < MC_KEEPALIVE_INTERVAL_MS / BSP_CLOCK_TICK_MS) return W5500_OK;

    rc = mc_send_keepalive(++session.keepalive_id);
    if (rc < 0) return rc;
    session.keepalive_at = clock_ticks();
    session.keepalive_pending = true;
    return W5500_OK;
}


int16_t mc_server_poll(void)
{
    uint16_t budget = MC_RX_BYTES_PER_POLL;

    while (budget--) {
        uint8_t byte;

        int16_t rc = w5500_tcp_recv(&byte, 1);
        if (rc < 0) return rc;
        if (rc == 0) break;

        led_toggle();
        rc = consume_byte(byte);

        if (rc < 0) return rc;
    }
    
    return poll_keepalive();
}
