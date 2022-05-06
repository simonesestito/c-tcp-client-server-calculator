#include "socket_utils.h"
#include "../common/logger.h"
#include "../common/main_init.h"
#include <strings.h>
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
    int new_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (new_socket_fd == -1) {
        log_errno(NULL, "Errore nella creazione della socket");
        return -1;
    }

    // Connetti al server
    if (connect(new_socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        log_errno(NULL, "Errore nella connect del socket");
        return -1;
    }

    return new_socket_fd;
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
    int reconnect_fd = -1;

    // Riprova a connetterti, se l'utente vuole ancora provarci (controlla "working")
    while (reconnect_fd <= 0 && working && delay_seconds < 10) {
        // Mostra animazione di attesa, se l'utente vuole ancora usarlo (controlla "working")
        for (unsigned int i = 0; i < delay_seconds * 8 && working; i++) {
            char loading_char;
            switch (i % 4) {
                case 0:
                    loading_char = '/';
                    break;
                case 1:
                    loading_char = '-';
                    break;
                case 2:
                    loading_char = '\\';
                    break;
                case 3:
                    loading_char = '|';
                    break;
            }
            wprintf(L" Tentativo di riconnessione tra %u secondi %-5c\r", delay_seconds - i / 8, loading_char);
            fflush(stdout);
            usleep(125000 /* 1/8 secondi */);
        }
        wprintf(L"Tentativo di riconnessione... %-15c\n", ' ');

        // Connessione...
        reconnect_fd = connect_to_server(ip, port);

        delay_seconds *= 2;
    }

    if (reconnect_fd <= 0) {
        // Non si riesce a connettersi nemmeno dopo svariate prove.
        // Ci si rinuncia.
        log_message(NULL, "Impossibile riconnettersi al server.\n");
    }

    return reconnect_fd;
}
