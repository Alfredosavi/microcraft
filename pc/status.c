#include "status.h"

#include "config.h"

#include "mc_packet.h"
#include "mc_send.h"

#include <string.h>
#include <stdio.h>



/**********************************************************************************
 * @brief Envia a resposta de status do servidor para um cliente
 *
 * Esta função envia um pacote de resposta contendo informações do servidor, como
 * versão do jogo, número máximo de jogadores, quantidade online e descrição.
 *
 * @param c Ponteiro para a estrutura do cliente que receberá a resposta.
 **********************************************************************************/
void status_send_response(Client *c)
{
    const char *json =
        "{"
        "\"version\":{\"name\":\"1.12.2\",\"protocol\":340},"
        "\"players\":{\"max\":1,\"online\":0},"
        "\"description\":{\"text\":\"TODO: descricao do server AQUI!\"}"
        "}";

    unsigned char buf[250]; // FIXME:
    int pos = 0;

    pos += mc_write_varint(buf + pos, 0); // packet id

    int len = strlen(json);
    pos += mc_write_varint(buf + pos, len);

    memcpy(buf + pos, json, len);
    pos += len;

    mc_send_packet(c->socket, buf, pos);

    DEBUG_PRINT("[SERVER] Status response enviado: %s\n", json);
}



/**********************************************************************************
 * @brief Trata uma requisição de ping e envia um pong
 *
 * Esta função processa um pacote de ping recebido de um cliente e envia de volta um
 * pacote pong com o mesmo payload, preservando a parte de dados após o ID.
 *
 * @param c       Ponteiro para a estrutura do cliente que enviou o ping.
 * @param data    Ponteiro para os dados recebidos no pacote de ping.
 * @param size    Tamanho total dos dados recebidos.
 * @param id_len  Comprimento do campo ID dentro do pacote.
 **********************************************************************************/
void status_handle_ping(Client *c, unsigned char *data, int size, int id_len)
{
    unsigned char buf[32];
    int pos = 0;

    pos += mc_write_varint(buf + pos, 1); // pong id

    int payload_size = size - id_len;
    memcpy((buf + pos), (data + id_len), payload_size);

    pos += payload_size;

    mc_send_packet(c->socket, buf, pos);

    DEBUG_PRINT("[SERVER] Pong enviado (payload %d bytes)\n", payload_size);
}
