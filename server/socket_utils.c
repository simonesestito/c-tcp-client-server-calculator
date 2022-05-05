#include "../common/socket_utils.h"
#include "../common/logger.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <strings.h>

/**
 * Crea il socket per il server, esegui il bind, metti in ascolto.
 *
 * Implementata solo nel programma SERVER.
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

    // Permetti di fare il bind sulla porta anche se ancora in TIME_WAIT (visibile da netstat)
    int enable = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        log_errno(NULL, "Errore in setsockopt(SO_REUSEADDR)");

    // Esegui il bind
    if (bind(socket_fd, (const struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        log_errno(NULL, "Errore nel bind del socket");
        return -1;
    }

    // Metti in ascolto
    if (listen(socket_fd, BACKLOG_SIZE) == -1) {
        log_errno(NULL, "Errore nella listen del socket");
        return -1;
    }

    return socket_fd;
}
