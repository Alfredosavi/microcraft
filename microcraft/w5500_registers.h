#ifndef W5500_REGISTERS_H
#define W5500_REGISTERS_H

#define W5500_CMD_OPEN       0x01
#define W5500_CMD_LISTEN     0x02
#define W5500_CMD_DISCON     0x08
#define W5500_CMD_CLOSE      0x10
#define W5500_CMD_SEND       0x20
#define W5500_CMD_RECV       0x40

#define W5500_IR_TIMEOUT     0x08
#define W5500_IR_SEND_OK     0x10

#define W5500_COMMON_GAR0     0x0001
#define W5500_COMMON_SUBR0    0x0005
#define W5500_COMMON_SHAR0    0x0009
#define W5500_COMMON_SIPR0    0x000f
#define W5500_COMMON_PHYCFGR  0x002e
#define W5500_COMMON_VERSIONR 0x0039

#define SN_MR          0x0000
#define SN_CR          0x0001
#define SN_IR          0x0002
#define SN_SR          0x0003
#define SN_PORT0       0x0004

#define SN_RXBUF_SIZE  0x001e
#define SN_TXBUF_SIZE  0x001f
#define SN_TX_FSR0     0x0020
#define SN_TX_WR0      0x0024
#define SN_RX_RSR0     0x0026
#define SN_RX_RD0      0x0028

#define BSB_COMMON_REG 0x00
#define BSB_S0_REG     0x01
#define BSB_S0_TX_BUF  0x02
#define BSB_S0_RX_BUF  0x03

#endif
