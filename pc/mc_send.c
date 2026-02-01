#include "mc_send.h"

#include "config.h"
#include "tools.h"

#include "mc_packet.h"

#include "tcp_server.h"

#include <stdio.h>
#include <string.h>

static unsigned char buffer[300]; // FIXME:



/**********************************************************************************
 * @brief Envia um pacote para um socket TCP
 *
 * Esta função envia um pacote de dados para um socket TCP, adicionando o tamanho do
 * pacote como varint no início.
 *
 * @param sock Descritor do socket para envio do pacote.
 * @param data Ponteiro para os dados do pacote.
 * @param len  Tamanho dos dados em bytes.
 **********************************************************************************/
void mc_send_packet(int sock, unsigned char *data, int len)
{
    int size_len = mc_write_varint(buffer, len);

    memcpy(buffer + size_len, data, len);

    tcp_write(sock, buffer, size_len + len);

    DEBUG_PRINT("[SERVER] Enviando pacote (%d bytes):\n", len);

    if (DEBUG) {
        dump_hex(data, len);
    }
}
