#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// MAC Address must be unicast (LSB of first byte must be 0).
#define APP_MAC_ADDRESS       {0x00, 0x12, 0x34, 0x56, 0x78, 0x90}

// IP Address, Subnet Mask, and Gateway Address must be valid for your network.
#define APP_IP_ADDRESS        {192, 168, 1, 111}
#define APP_SUBNET_MASK       {255, 255, 255, 0}
#define APP_GATEWAY_ADDRESS   {192, 168, 1, 1}

#define APP_TCP_PORT          25565 // Default port for Minecraft servers
#define APP_POLL_INTERVAL_MS  1
#define APP_RETRY_DELAY_MS    1000

#define APP_CLOSE_WAIT_MS     5000

#endif
