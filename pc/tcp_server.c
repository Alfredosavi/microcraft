#include "tcp_server.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>



/**********************************************************************************
 * @brief Cria e configura um socket de servidor TCP
 *
 * Esta função cria um socket TCP, configura a opção SO_REUSEADDR, associa-o a uma
 * porta específica e inicia a escuta por conexões de clientes.
 *
 * @param port Porta na qual o servidor TCP irá escutar por conexões.
 * @return     Descritor do socket do servidor em caso de sucesso, ou -1 em caso de
 *             erro durante criação, bind ou listen.
 **********************************************************************************/
int tcp_server_create(int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (fd < 0) {
        return -1;
    }

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        return -1;
    }

    if (listen(fd, 5) < 0) {
        return -1;
    }

    return fd;
}



/**********************************************************************************
 * @brief Aceita uma conexão de cliente em um servidor TCP
 *
 * Esta função aguarda e aceita uma conexão de entrada em um socket de servidor TCP.
 * Retorna um descritor de socket para comunicação com o cliente.
 *
 * @param server_fd Descritor do socket de servidor que está aguardando conexões.
 * @return          Descritor do socket do cliente em caso de sucesso, ou -1 em caso
 *                  de erro.
 **********************************************************************************/
int tcp_server_accept(int server_fd)
{
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    return accept(server_fd, (struct sockaddr *)&client_addr, &len);
}



/**********************************************************************************
 * @brief Recebe dados de um socket TCP
 *
 * Esta função lê dados de um socket TCP, garantindo que todos os bytes solicitados
 * sejam recebidos. Em caso de erro ou se a conexão for encerrada, retorna o valor
 * retornado por recv().
 *
 * @param sock    Descritor do socket para leitura.
 * @param buffer  Ponteiro para o buffer onde os dados serão armazenados.
 * @param size    Número de bytes a ler.
 * @return        Total de bytes lidos em caso de sucesso, ou valor negativo em caso
 *                de erro ou encerramento da conexão.
 **********************************************************************************/
int tcp_read(int sock, void *buffer, size_t size)
{
    size_t total = 0;

    while (total < size) {
        int r = recv(sock, (char *)buffer + total, (size - total), 0);

        if (r <= 0) {
            return r;
        }

        total += r;
    }

    return total;
}


/**********************************************************************************
 * @brief Envia dados através de um socket TCP
 *
 * Esta função envia todo o conteúdo de um buffer para um socket TCP, garantindo que
 * todos os bytes sejam transmitidos, mesmo que múltiplas chamadas a send() sejam
 * necessárias. Em caso de erro, retorna o valor retornado por send().
 *
 * @param sock    Descritor do socket para envio.
 * @param buffer  Ponteiro para os dados a serem enviados.
 * @param size    Número de bytes a enviar.
 * @return        Total de bytes enviados em caso de sucesso, ou valor negativo em
 *                caso de erro.
 **********************************************************************************/
int tcp_write(int sock, const void *buffer, size_t size)
{
    size_t total = 0;

    while (total < size) {
        int r = send(sock, (char *)buffer + total, (size - total), 0);

        if (r <= 0) {
            return r;
        }

        total += r;
    }

    return total;
}



/**********************************************************************************
 * @brief Encerra a conexão de um socket TCP
 *
 * Esta função fecha um socket TCP específico utilizando a chamada de sistema close().
 *
 * @param sock Descritor do socket a ser fechado.
 **********************************************************************************/
void tcp_server_close(int sock)
{
    close(sock);
}
