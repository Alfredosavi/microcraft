#include <stdio.h>
#include <stdlib.h>

#include "config.h"

#include "tcp_server.h"
#include "mc_packet.h"
#include "client.h"

#define SERVER_PORT 25565
int DEBUG = 1;


int main(void)
{
    int server = tcp_server_create(SERVER_PORT);
    if (server < 0) {
        fprintf(stderr, "[SERVER] Erro ao criar servidor TCP\n");
        return 1;
    }

    printf("[SERVER] Servidor rodando na porta %d...\n", SERVER_PORT);

    while (1) {
        int sock = tcp_server_accept(server);

        if (sock < 0) {
            fprintf(stderr, "[SERVER] Erro ao aceitar conexão do cliente\n");
            continue;
        }

        Client client;
        client_init(&client, sock);

        DEBUG_PRINT("[SERVER] Cliente conectado, socket=%d\n", sock);

        while (1) {
            int size = 0;
            unsigned char *packet = mc_read_packet(sock, &size);

            if (packet == NULL) {
                DEBUG_PRINT("[CLIENT] Cliente desconectou ou erro ao ler pacote\n");
                break;
            }

            DEBUG_PRINT("[CLIENT] Pacote recebido: size=%d\n", size);
            client_handle_packet(&client, packet, size);
            free(packet);
        }

        tcp_server_close(sock);
        DEBUG_PRINT("[SERVER] Cliente desconectado, socket=%d\n", sock);
    }
}
