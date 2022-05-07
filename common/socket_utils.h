#ifndef SERVER_SOCKET_UTILS_H
#define SERVER_SOCKET_UTILS_H

#include <netinet/in.h>
#include <stdio.h>

#define DEFAULT_PORT 12345
#define DEFAULT_HOST "127.0.0.1"
#define BACKLOG_SIZE 128
#define SERVER_ERROR_MESSAGE_PREFIX '-'

/**
 * Raccogli le informazioni del client
 */
struct sock_info {
    /**
     * File pointer del socket col client.
     * Utile per usare funzioni di libreria come getline
     * che richiedono un FILE* e non un int file descriptor.
     */
    FILE *socket_file;

    /**
     * Informazioni aggiuntive sul socket
     */
    struct sockaddr_in client_info;
};

/**
 * Converti una stringa in un numero intero senza segno a 16 bit
 * @return Numero convertito, o 0 in caso di errore
 */
uint16_t str_to_uint16(const char *str);

/**
 * Elimina il \n o \r\n finali di una stringa ottenuta da una socket da getline.
 * @param line Linea da cui rimuovere i caratteri
 * @param size Dimensione della linea, inclusi questi caratteri
 */
void strip_newline(char *line, ssize_t *size);

#endif //SERVER_SOCKET_UTILS_H
