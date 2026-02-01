#include "play.h"

#include "config.h"

#include "mc_packet.h"
#include "mc_send.h"

#include "client.h"

#include <string.h>
#include <stdio.h>



/**********************************************************************************
 * @brief Escreve um valor double em formato big-endian em um buffer
 *
 * Esta função converte um valor double para a representação em bytes no formato
 * big-endian e armazena os 8 bytes resultantes no buffer fornecido.
 *
 * @param buf Ponteiro para o buffer onde os bytes serão armazenados (tamanho mínimo 8).
 * @param val Valor double a ser convertido e armazenado.
 **********************************************************************************/
static void write_double_be(unsigned char *buf, double val)
{
    unsigned char *p = (unsigned char *)&val;

    for (int i=0; i<8; i++) {
        buf[i] = p[7 - i];
    }
}



/**********************************************************************************
 * @brief Escreve um valor float em formato big-endian em um buffer
 *
 * Esta função converte um valor float para a representação em bytes no formato
 * big-endian e armazena os 4 bytes resultantes no buffer fornecido.
 *
 * @param buf Ponteiro para o buffer onde os bytes serão armazenados (tamanho mínimo 4).
 * @param val Valor float a ser convertido e armazenado.
 **********************************************************************************/
static void write_float_be(unsigned char *buf, float val)
{
    unsigned char *p = (unsigned char *)&val;

    for (int i=0; i<4; i++) {
        buf[i] = p[3 - i];
    }
}



/**********************************************************************************
 * @brief Envia o pacote de Join Game para um cliente
 *
 * Esta função envia um pacote Join Game, configurando informações como ID da
 * entidade, modo de jogo, dimensão, dificuldade, número máximo de jogadores e
 * tipo de mundo.
 *
 * @param c Ponteiro para a estrutura do cliente que receberá o pacote.
 **********************************************************************************/
void play_send_join_game(Client *c)
{
    unsigned char buf[100]; // FIXME:
    int pos = 0;

    pos += mc_write_varint(buf + pos, 0x23); // Join Game ID (1.12)

    // Entity ID
    int entity_id = 1;
    memcpy(buf + pos, &entity_id, 4);
    pos += 4;

    // Gamemode (survival)
    buf[pos++] = 0;

    // Dimension (overworld)
    int dimension = 0;
    memcpy(buf + pos, &dimension, 4);
    pos += 4;

    // Difficulty
    buf[pos++] = 1;

    // Max players
    buf[pos++] = 1;

    // Level type "default"
    pos += mc_write_string(buf + pos, "default");

    // Reduced debug info
    buf[pos++] = 0;

    mc_send_packet(c->socket, buf, pos);

    DEBUG_PRINT("[SERVER] Join Game enviado\n");
}



/**********************************************************************************
 * @brief Envia a posição e orientação do jogador para um cliente
 *
 * Esta função envia um pacote Player Position And Look, definindo coordenadas
 * (x, y, z), yaw, pitch, flags e ID de teletransporte para o cliente especificado.
 *
 * @param c Ponteiro para a estrutura do cliente que receberá o pacote.
 **********************************************************************************/
void play_send_player_pos(Client *c)
{
    unsigned char buf[256]; // FIXME:
    int pos = 0;

    pos += mc_write_varint(buf + pos, 0x2F); // Player Position And Look

    double x = 0.0;
    double y = 70.0;
    double z = 0.0;
    float yaw = 0;
    float pitch = 0;

    write_double_be(buf + pos, x);
    pos += 8;
    write_double_be(buf + pos, y);
    pos += 8;
    write_double_be(buf + pos, z);
    pos += 8;

    write_float_be(buf + pos, yaw);
    pos += 4;
    write_float_be(buf + pos, pitch);
    pos += 4;

    buf[pos++] = 0; // flags
    pos += mc_write_varint(buf + pos, 1); // teleport ID

    mc_send_packet(c->socket, buf, pos);

    DEBUG_PRINT("[SERVER] Player Position enviado\n");
}
