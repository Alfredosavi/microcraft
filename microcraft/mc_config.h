#ifndef MC_CONFIG_H
#define MC_CONFIG_H

#define MC_PROTOCOL_VERSION 340u
#define MC_STATUS_JSON \
    "{\"version\":{\"name\":\"1.12.2\",\"protocol\":340}," \
    "\"players\":{\"max\":1,\"online\":0}," \
    "\"description\":{\"text\":\"Minecraft Server | Powered by MSP430G2553\"}}"

#define MC_MAX_PACKET_BYTES 2097151u
#define MC_RX_BYTES_PER_POLL 64u
#define MC_TX_WAIT_MS 1000u
#define MC_KEEPALIVE_INTERVAL_MS 10000u
#define MC_KEEPALIVE_TIMEOUT_MS 30000u
#define MC_USERNAME_BYTES 16u   // Limit of the username in the Java 1.12.2 protocol.

#endif
