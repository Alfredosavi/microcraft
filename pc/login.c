#include "login.h"

#include "config.h"

#include "mc_packet.h"
#include "mc_send.h"

#include "play.h"

#include <string.h>
#include <stdio.h>



static int read_varint_buf(unsigned char *buf, int *value, int *consumed);
static void send_login_success(Client *c, const char *name);



/**********************************************************************************
 * @brief Lê um valor VarInt de um buffer de memória
 *
 * Esta função decodifica um valor VarInt presente em um buffer de bytes, retornando
 * o valor lido e o número de bytes consumidos. Em caso de VarInt inválido, retorna -1.
 *
 * @param buf      Ponteiro para o buffer contendo os bytes a serem lidos.
 * @param value    Ponteiro para a variável onde o valor decodificado será armazenado.
 * @param consumed Ponteiro para variável que receberá o número de bytes consumidos.
 * @return         0 em caso de sucesso, ou -1 se o VarInt for inválido ou muito grande.
 **********************************************************************************/
static int read_varint_buf(unsigned char *buf, int *value, int *consumed)
{
    int numRead = 0;
    int result = 0;
    unsigned char read;

    do {
        read = buf[numRead];
        int val = (read & 0x7F);
        result |= val << (7 * numRead);

        numRead++;
        if (numRead > 5) {
            DEBUG_PRINT("[CLIENT] Erro: VarInt muito grande\n");
            return -1;
        }

    } while (read & 0x80);

    *value = result;
    *consumed = numRead;

    DEBUG_PRINT("[CLIENT] VarInt lido: %d (bytes=%d)\n", result, numRead);
    return 0;
}



/**********************************************************************************
 * @brief Envia um pacote de Login Success para o cliente
 *
 * Esta função envia um pacote indicando que o login do jogador foi bem-sucedido,
 * incluindo UUID fixo e o nome do jogador.
 *
 * @param c    Ponteiro para a estrutura do cliente que receberá o pacote.
 * @param name Nome do jogador que realizou o login.
 **********************************************************************************/
static void send_login_success(Client *c, const char *name)
{
    unsigned char buf[50];
    int pos = 0;

    const char *uuid = "00000000-0000-0000-0000-000000000000";

    pos += mc_write_varint(buf + pos, 0x02);      // Packet ID = Login Success
    pos += mc_write_string(buf + pos, uuid);      // UUID
    pos += mc_write_string(buf + pos, name);      // Nome do jogador

    mc_send_packet(c->socket, buf, pos);

    DEBUG_PRINT("[SERVER] Login success enviado para jogador '%s'\n", name);
}



/**********************************************************************************
 * @brief Trata o pacote LOGIN_START de um cliente
 *
 * Esta função processa o pacote de início de login recebido de um cliente,
 * extrai o nome do jogador, envia o pacote de Login Success e altera o estado do
 * cliente para PLAY, enviando pacotes de Join Game e Player Position.
 *
 * @param c      Ponteiro para a estrutura do cliente que enviou o pacote.
 * @param data   Ponteiro para os dados recebidos no pacote.
 * @param size   Tamanho total dos dados recebidos.
 * @param id_len Comprimento do campo ID dentro do pacote.
 **********************************************************************************/
void login_handle_start(Client *c, unsigned char *data, int size, int id_len)
{
    int offset = id_len;
    int name_len;
    int consumed;

    if (read_varint_buf(data + offset, &name_len, &consumed) != 0) {
        return;
    }

    offset += consumed;

    if (name_len <= 0 || name_len > 64) {
        return;
    }

    char name[65];
    memcpy(name, data + offset, name_len);
    name[name_len] = 0;

    DEBUG_PRINT("[SERVER] Pacote LOGIN_START recebido: jogador='%s'\n", name);

    send_login_success(c, name);

    c->state = STATE_PLAY;
    DEBUG_PRINT("[CLIENT] Estado alterado para PLAY\n");

    play_send_join_game(c);
    play_send_player_pos(c);
}
