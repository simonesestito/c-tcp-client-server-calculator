#include "socket_utils.h"
#include "../common/logger.h"
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

/**
 * Connettiti al server.
 *
 * Implementata solo nel programma CLIENT.
 *
 * @param ip Indirizzo IP del server
 * @param port Porta del server
 * @return -1 in caso di errore, il file descriptor del server socket altrimenti
 */
int connect_to_server(const char *ip, uint16_t port) {
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address)); // Azzera la struct
    server_address.sin_port = htons(port); // Imposta la porta
    server_address.sin_family = AF_INET; // Imposta IPv4

    // Prova a impostare l'indirizzo IP
    if (inet_pton(AF_INET, ip, &(server_address.sin_addr)) != 1) {
        log_message(NULL, "ERRORE: Indirizzo IP invalido\n");
        return -1;
    }

    // Crea la socket (unnamed)
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        log_errno(NULL, "Errore nella creazione della socket");
        return -1;
    }

    // Connetti al server
    if (connect(socket_fd, (struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
        log_errno(NULL, "Errore nella connect del socket");
        return -1;
    }

    return socket_fd;
}

/**
 * Riconnettiti al server, eseguendo un backoff esponenziale.
 *
 * Implementata solo nel programma CLIENT.
 *
 * @param ip Indirizzo IP del server
 * @param port Porta del server
 * @return -1 in caso di errore per più di 3 volte, il file descriptor del server socket altrimenti
 */
int reconnect_exponential(const char *ip, uint16_t port) {
    unsigned int delay_seconds = 1;
    int socket_fd = 0;

    // Riprova a connetterti
    while (socket_fd <= SOCKET_WILL_RETRY && delay_seconds < 10) {
        wprintf(L"Tentativo di riconnessione dopo %u secondi\n", delay_seconds);
        // TODO: Fare animazione di attesa [===>   ]
        sleep(delay_seconds);

        // Connessione...
        socket_fd = connect_to_server(ip, port);

        delay_seconds *= 2;
    }

    if (socket_fd <= SOCKET_WILL_RETRY) {
        // Non si riesce a connettersi nemmeno dopo svariate prove.
        // Ci si rinuncia.
        log_message(NULL, "Impossibile riconnettersi al server.\n");
    }

    return socket_fd;
}