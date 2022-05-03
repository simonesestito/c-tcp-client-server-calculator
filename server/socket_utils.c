#include "socket_utils.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Converti una stringa in un numero intero senza segno a 16 bit
 * @return Numero convertito, o 0 in caso di errore
 */
uint16_t str_to_uint16(const char *str) {
    char *string_part;
    long number = strtol(str, &string_part, 10);

    uint16_t parsed_number;
    if (errno != 0 || string_part[0] != '\0' || number <= 0 || number > 65535)
        parsed_number = 0;
    else
        parsed_number = (uint16_t) number;

    // Reset errno dopo di strtol()
    errno = 0;

    return parsed_number;
}

/**
 * Crea il socket per il server, esegui il bind, metti in ascolto.
 *
 * @param ip Indirizzo IP del server
 * @param port Porta del server
 * @return -1 in caso di errore, il file descriptor del server socket altrimenti
 */
int bind_server(const char *ip, uint16_t port) {
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address)); // Azzera la struct
    server_address.sin_port = htons(port); // Imposta la porta
    server_address.sin_family = AF_INET; // Imposta IPv4

    // Prova ad impostare l'indirizzo IP
    if (inet_pton(AF_INET, ip, &(server_address.sin_addr)) != 1) {
        fprintf(stderr, "ERRORE: Indirizzo IP invalido\n");
        return -1;
    }

    // Crea la socket (unnamed)
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Errore nella creazione della socket");
        return -1;
    }

    // Esegui il bind
    if (bind(socket_fd, (const struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
        perror("Errore nel bind del socket");
        return -1;
    }

    // Metti in ascolto
    if (listen(socket_fd, BACKLOG_SIZE) == -1) {
        perror("Errore nella listen del socket");
        return -1;
    }

    return socket_fd;
}