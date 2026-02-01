#include "client.h"

#include "config.h"

#include "mc_packet.h"
#include "mc_send.h"

#include "status.h"
#include "login.h"

#include <stdio.h>
#include <string.h>



/**********************************************************************************
 * @brief Lê um valor VarInt de um buffer de bytes
 *
 * Esta função decodifica um VarInt presente em um buffer de memória, retornando
 * o valor lido e o número de bytes consumidos. Retorna -1 se o VarInt for inválido
 * ou exceder o tamanho máximo permitido.
 *
 * @param buf   Ponteiro para o buffer contendo os bytes a serem lidos.
 * @param value Ponteiro para variável que receberá o valor decodificado.
 * @param bytes Ponteiro para variável que receberá o número de bytes consumidos.
 * @return      0 em caso de sucesso, ou -1 em caso de erro.
 **********************************************************************************/
static int read_varint_from_buffer(unsigned char *buf, int *value, int *bytes)
{
    int numRead = 0;
    int result = 0;
    unsigned char read = 0;

    do {
        read = buf[numRead];
        int val = (read & 0x7F);
        result |= val << (7 * numRead);

        numRead++;
        if (numRead > 5) {
            return -1;
        }

    } while (read & 0x80);

    *value = result;
    *bytes = numRead;

    DEBUG_PRINT("[CLIENT] VarInt lido: %d (bytes=%d)\n", result, numRead);
    return 0;
}



/**********************************************************************************
 * @brief Inicializa uma estrutura de cliente
 *
 * Esta função configura um cliente atribuindo o descritor de socket e definindo o
 * estado inicial como HANDSHAKE.
 *
 * @param c    Ponteiro para a estrutura do cliente a ser inicializada.
 * @param sock Descritor de socket associado ao cliente.
 **********************************************************************************/
void client_init(Client *c, int sock)
{
    c->socket = sock;
    c->state = STATE_HANDSHAKE;
    DEBUG_PRINT("[CLIENT] Cliente inicializado, sock=%d, estado=HANDSHAKE\n", sock);
}


/**********************************************************************************
 * @brief Processa um pacote recebido de um cliente
 *
 * Esta função interpreta e trata pacotes recebidos de um cliente, dependendo do
 * estado atual do cliente (HANDSHAKE, STATUS ou LOGIN), executando ações como
 * atualização de estado, envio de respostas STATUS, PONG ou iniciando o login.
 *
 * @param c    Ponteiro para a estrutura do cliente que enviou o pacote.
 * @param data Ponteiro para os dados recebidos no pacote.
 * @param size Tamanho total dos dados recebidos.
 **********************************************************************************/
void client_handle_packet(Client *c, unsigned char *data, int size)
{
    int packet_id;
    int id_len;

    if (read_varint_from_buffer(data, &packet_id, &id_len) != 0) {
        DEBUG_PRINT("[CLIENT] Erro ao ler PacketID\n");
        return;
    }

    DEBUG_PRINT("[CLIENT] State=%d PacketID=%d Size=%d\n", c->state, packet_id, size);

    // ===== HANDSHAKE =====
    if (c->state == STATE_HANDSHAKE && packet_id == 0) {
        int offset = id_len;
        int dummy = 0, consumed = 0;

        // Protocol version
        read_varint_from_buffer(data + offset, &dummy, &consumed);
        offset += consumed;

        // Server address length
        read_varint_from_buffer(data + offset, &dummy, &consumed);
        offset += consumed + dummy;

        // Porta
        offset += 2;

        // Next state
        int next_state = 0;
        read_varint_from_buffer(data + offset, &next_state, &consumed);

        if (next_state == 1) {
            c->state = STATE_STATUS;
            DEBUG_PRINT("[CLIENT] Mudou para STATUS\n");
        } else if (next_state == 2) {
            c->state = STATE_LOGIN;
            DEBUG_PRINT("[CLIENT] Mudou para LOGIN\n");
        }
        return;
    }

    // ===== STATUS =====
    if (c->state == STATE_STATUS) {
        if (packet_id == 0) {
            DEBUG_PRINT("[SERVER] Enviando resposta STATUS\n");
            status_send_response(c);
        } else if (packet_id == 1) {
            unsigned char buf[16];
            int pos = 0;

            pos += mc_write_varint(buf + pos, 1); // Pong ID
            memcpy(buf + pos, data + id_len, 8);
            pos += 8;

            mc_send_packet(c->socket, buf, pos);
            DEBUG_PRINT("[SERVER] Pong enviado (8 bytes do ping copiados)\n");
        }
    }

    // ===== LOGIN =====
    if (c->state == STATE_LOGIN) {
        if (packet_id == 0) {
            DEBUG_PRINT("[SERVER] Pacote LOGIN_START recebido\n");
            login_handle_start(c, data, size, id_len);
        }
    }
}
