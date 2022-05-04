#ifndef SERVER_SOCKET_UTILS_H
#define SERVER_SOCKET_UTILS_H

#include <netinet/in.h>
#include <stdio.h>

#define DEFAULT_PORT 12345
#define DEFAULT_HOST "127.0.0.1"
#define BACKLOG_SIZE 128

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
 * Crea il socket per il server, esegui il bind, metti in ascolto.
 *
 * @param ip Indirizzo IP del server
 * @param port Porta del server
 * @return -1 in caso di errore, il file descriptor del server socket altrimenti
 */
int bind_server(const char *ip, uint16_t port);

#endif //SERVER_SOCKET_UTILS_H
