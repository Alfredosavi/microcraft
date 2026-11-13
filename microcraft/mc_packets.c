#include "mc_packets.h"

#include "mc_config.h"
#include "w5500.h"
#include "bsp_delay.h"



static const char status_json[] = MC_STATUS_JSON;

// ID Login Success + sizeof(UUID) + UUID. UUID=0 para singleplayer offline.
static const uint8_t login_identity[] = "\x02\x24" "00000000-0000-0000-0000-000000000000";
static const uint8_t pong_header[] = {9, 1};

/* Protocol 340 Position: X:26, Y:12, Z:26 bits, big-endian. */
const uint8_t mc_lever_position[8] = {0, 0, 2, 1, 0x18, 0, 0, 10}; /* (8,70,10) */
static const uint8_t lamp_position[8] = {0, 0, 2, 0x41, 0x18, 0, 0, 10}; /* (9,70,10) */

  /* Join Game and Position And Look, including lengths, in big-endian.
   * Entity 1, survival, overworld, easy, default; position (8.5, 70, 8.5).
  * Y=70 in IEEE754: 0x4051800000000000; flags=0 and teleport ID=1. */
static const uint8_t play_start[] = {
    21, 0x23, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1,
    7, 'd', 'e', 'f', 'a', 'u', 'l', 't', 0,
};

static const uint8_t spawn_position[] = {
    35, 0x2f,
    0x40, 0x21, 0, 0, 0, 0, 0, 0,
    0x40, 0x51, 0x80, 0, 0, 0, 0, 0,
    0x40, 0x21, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, MC_INITIAL_TELEPORT_ID
};

// Repeated chunk data lives in Flash, because RAM is limited xdx
#define REPEAT8(x) x, x, x, x, x, x, x, x
#define REPEAT64(x) REPEAT8(x), REPEAT8(x), REPEAT8(x), REPEAT8(x), \
                    REPEAT8(x), REPEAT8(x), REPEAT8(x), REPEAT8(x)
static const uint8_t chunk_zero[128] = {0};
static const uint8_t chunk_stone[128] = {REPEAT64(0x11), REPEAT64(0x11)};
/* Layer Y=70: lever at (8,70,10), unlit lamp at (9,70,10).
 * Each row is one big-endian long; X=8/9 occupy its fourth byte.
 * Palette indices 2 (low nibble) and 3 (high nibble). */
static const uint8_t chunk_devices[128] = {[10u * 8u + 3u] = 0x32};
static const uint8_t chunk_sky[128] = {REPEAT64(0xff), REPEAT64(0xff)};
static const uint8_t chunk_biomes[128] = {REPEAT64(1), REPEAT64(1)};
#undef REPEAT64
#undef REPEAT8


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


int16_t mc_send_keepalive(uint16_t id)
{
    /* Protocol 340: packet 0x1f, followed by a big-endian 64-bit Long. */
    uint8_t packet[10] = {9, 0x1F, 0, 0, 0, 0, 0, 0, 0, 0};
    packet[8] = (uint8_t)(id >> 8);
    packet[9] = (uint8_t)id;
    return send_all(packet, sizeof(packet));
}


static int16_t send_block_change(const uint8_t position[8], uint16_t state)
{
    /* Both device states occupy two VarInt bytes. */
    static const uint8_t header[] = {11, 0x0b};
    int16_t rc = send_all(header, sizeof(header));
    if (rc < 0) return rc;
    rc = send_all(position, 8);
    return rc < 0 ? rc : send_varint(state);
}


int16_t mc_send_devices(bool powered)
{
    /* Lever metadata: floor orientation 5, powered bit 8. Lamp IDs: 123/124. */
    int16_t rc = send_block_change(mc_lever_position, (69u << 4) | 5u | (powered ? 8u : 0u));
    return rc < 0 ? rc : send_block_change(lamp_position, (powered ? 124u : 123u) << 4);
}


static int16_t send_device_chat(bool powered)
{
    static const char on[] = MC_LED_ON_JSON;
    static const char off[] = MC_LED_OFF_JSON;

    const char *text = powered ? on : off;
    uint16_t length = powered ? sizeof(on) - 1u : sizeof(off) - 1u;

    int16_t rc = send_varint(1u + varint_size(length) + length + 1u);
    if (rc < 0) return rc;

    rc = send_varint(0x0f); // Chat Message, protocol 340.
    if (rc < 0) return rc;

    rc = send_varint(length);
    if (rc < 0) return rc;

    rc = send_all((const uint8_t *)text, length);
    return rc < 0 ? rc : send_varint(1); // System message in the chat area.
}


static int16_t send_lever_sound(bool powered)
{
    static const char name[] = "block.lever.click";
    /* Blocks category; center (8.5,70.5,10.5), coordinates multiplied by 8.
     * Volume 0.3, pitch 0.6 when on and 0.5 when off (big-endian floats). */
    static const uint8_t settings[] = {
        4,
        0, 0, 0, 68, 0, 0, 2, 52, 0, 0, 0, 84,
        0x3e, 0x99, 0x99, 0x9a
    };

    static const uint8_t pitch_on[] = {0x3f, 0x19, 0x99, 0x9a};
    static const uint8_t pitch_off[] = {0x3f, 0, 0, 0};

    const uint16_t length = sizeof(name) - 1u;
    
    int16_t rc = send_varint(1u + varint_size(length) + length + sizeof(settings) + 4u);
    if (rc < 0) return rc;
    
    rc = send_varint(0x19); /* Named Sound Effect, protocol 340. */
    if (rc < 0) return rc;
    
    rc = send_varint(length);
    if (rc < 0) return rc;
    
    rc = send_all((const uint8_t *)name, length);
    if (rc < 0) return rc;
    
    rc = send_all(settings, sizeof(settings));
    return rc < 0 ? rc : send_all(powered ? pitch_on : pitch_off, 4);
}


int16_t mc_send_device_feedback(bool powered)
{
    int16_t rc = send_device_chat(powered);
    return rc < 0 ? rc : send_lever_sound(powered);
}


static int16_t send_chunk_runs(const uint8_t *data, uint8_t count)
{
    while (count--) {
        int16_t rc = send_all(data, 128);
        if (rc < 0) return rc;
    }
    
    return W5500_OK;
}


static int16_t send_spawn_chunk(void)
{
    /* Chunk (0,0), ground-up, section Y=4 only (world Y=64..79).
     * Palette: air, stone, floor-mounted lever (off), redstone lamp (off).
     * 4096 blocks at 4 bits = 256 longs.
     * Data: 10-byte section header + 2048 blocks + 4096 light + 256 biomes. */
    static const uint8_t header[] = {
        0x20, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0x10
    };

    /* Legacy states: lever=(69<<4)|5=1109, unlit lamp=123<<4=1968. */
    static const uint8_t section[] = {4, 4, 0, 16, 0xd5, 0x08, 0xb0, 0x0f, 0x80, 2};
    const uint16_t data_size = 6410u;

    int16_t rc = send_varint(sizeof(header) + varint_size(data_size) + data_size + 1u);
    if (rc < 0) return rc;

    rc = send_all(header, sizeof(header));
    if (rc < 0) return rc;

    rc = send_varint(data_size);
    if (rc < 0) return rc;

    rc = send_all(section, sizeof(section));
    if (rc < 0) return rc;

    /* Y-major order: five air layers, stone, devices, nine air layers. */
    rc = send_chunk_runs(chunk_zero, 5);
    if (rc < 0) return rc;

    rc = send_chunk_runs(chunk_stone, 1);
    if (rc < 0) return rc;

    rc = send_chunk_runs(chunk_devices, 1);
    if (rc < 0) return rc;

    rc = send_chunk_runs(chunk_zero, 9);
    if (rc < 0) return rc;

    rc = send_chunk_runs(chunk_zero, 16); /* Block light. */
    if (rc < 0) return rc;

    rc = send_chunk_runs(chunk_zero, 6); /* Sky light below/inside the platform. */
    if (rc < 0) return rc;

    rc = send_chunk_runs(chunk_sky, 10); /* Full sky light above the platform. */
    if (rc < 0) return rc;

    rc = send_chunk_runs(chunk_biomes, 2); /* Plains. */
    return rc < 0 ? rc : send_varint(0); /* No block entities. */
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
    
    rc = send_all(play_start, sizeof(play_start));
    if (rc < 0) return rc;

    rc = send_spawn_chunk();
    if (rc < 0) return rc;

    return send_all(spawn_position, sizeof(spawn_position));
}
