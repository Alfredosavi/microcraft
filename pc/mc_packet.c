#include "mc_packet.h"

#include "tcp_server.h"

#include <stdlib.h>
#include <string.h>



/**********************************************************************************
 * @brief Lê um valor VarInt de um socket TCP
 *
 * Esta função lê um valor VarInt do socket fornecido, decodificando os bytes
 * recebidos de acordo com a especificação VarInt do protocolo Minecraft.
 *
 * @param sock  Descritor do socket de onde os dados serão lidos.
 * @param value Ponteiro para a variável onde o valor lido será armazenado.
 * @return      Número de bytes lidos em caso de sucesso, ou -1 em caso de erro.
 **********************************************************************************/
int mc_read_varint(int sock, int *value) {
    int numRead = 0;
    int result = 0;
    unsigned char read;

    do {
        if (tcp_read(sock, &read, 1) <= 0) {
            return -1;
        }

        int val = (read & 0x7F);
        result |= val << (7 * numRead);

        numRead++;
        if (numRead > 5) {
            return -1;
        }

    } while (read & 0x80);

    *value = result;
    return numRead;
}



/**********************************************************************************
 * @brief Escreve um valor VarInt em um buffer
 *
 * Esta função codifica um inteiro no formato VarInt do protocolo Minecraft e armazena
 * os bytes resultantes no buffer fornecido.
 *
 * @param buf   Ponteiro para o buffer onde os bytes serão armazenados.
 * @param value Valor inteiro a ser codificado como VarInt.
 * @return      Número de bytes escritos no buffer.
 **********************************************************************************/
int mc_write_varint(unsigned char *buf, int value) {
    int pos = 0;

    do {
        unsigned char temp = value & 0x7F;
        value >>= 7;

        if (value) {
            temp |= 0x80;
        }

        buf[pos++] = temp;
    } while (value);

    return pos;
}



/**********************************************************************************
 * @brief Lê um pacote completo de um socket TCP
 *
 * Esta função lê o tamanho do pacote como VarInt e em seguida lê os bytes do pacote
 * do socket TCP, alocando memória para armazenar os dados.
 *
 * @param sock        Descritor do socket de onde os dados serão lidos.
 * @param packet_size Ponteiro para variável que receberá o tamanho do pacote lido.
 * @return            Ponteiro para o buffer com os dados do pacote, ou NULL em caso
 *                    de erro ou falha de alocação.
 **********************************************************************************/
unsigned char *mc_read_packet(int sock, int *packet_size) {
    int len;

    if (mc_read_varint(sock, &len) <= 0) {
        return NULL;
    }

    unsigned char *data = malloc(len);
    if (!data) {
        return NULL;
    }

    if (tcp_read(sock, data, len) <= 0) {
        free(data);
        return NULL;
    }

    *packet_size = len;
    return data;
}



/**********************************************************************************
 * @brief Escreve uma string no formato do protocolo Minecraft em um buffer
 *
 * Esta função codifica uma string, primeiro escrevendo seu comprimento como VarInt
 * e em seguida copiando os caracteres da string para o buffer.
 *
 * @param buf Ponteiro para o buffer onde a string será escrita.
 * @param str Ponteiro para a string a ser codificada.
 * @return    Número total de bytes escritos no buffer.
 **********************************************************************************/
int mc_write_string(unsigned char *buf, const char *str)
{
    int len = strlen(str);
    int pos = 0;

    pos += mc_write_varint(buf + pos, len);
    memcpy(buf + pos, str, len);

    return pos + len;
}
