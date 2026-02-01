#include "tools.h"

#include <stdio.h>


/**********************************************************************************
 * @brief Exibe o conteúdo de um buffer em formato hexadecimal
 *
 * Esta função percorre um buffer de bytes e imprime cada valor em hexadecimal,
 * 16 bytes por linha.
 *
 * @param buf  Ponteiro para o buffer de bytes a ser exibido.
 * @param size Número de bytes no buffer.
 **********************************************************************************/
void dump_hex(unsigned char *buf, int size)
{
    for (int i=0; i<size; i++) {
        printf("%02X ", buf[i]);

        if ((i % 16) == 15) {
            printf("\n");
        }
    }

    printf("\n");
}
