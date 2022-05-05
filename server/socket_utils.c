#include "socket_utils.h"
#include "logger.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <strings.h>
#include <stdlib.h>
#include <poll.h>

/**
 * Converti una stringa in un numero intero senza segno a 16 bit
 * @return Numero convertito, o 0 in caso di errore
 */
uint16_t str_to_uint16(const char *str) {
    char *string_part;
    long number = strtol(str, &string_part, 10);

    uint16_t parsed_number;
    if (string_part[0] != '\0' || number <= 0 || number > 65535)
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

    // Prova a impostare l'indirizzo IP
    if (inet_pton(AF_INET, ip, &(server_address.sin_addr)) != 1) {
        log_message(NULL, L"ERRORE: Indirizzo IP invalido\n");
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

/**
 * Resta in attesa di dati sul file descriptor.
 *
 * Utilizza poll()
 * (e non select(), per via delle limitazioni scritte nel man)
 * per:
 * - avere subito dati, se arrivano
 * - ogni intervallo di tempo, controllare se il running_flag
 *   indica ancora uno stato di esecuzione;
 *   in caso contrario, termina l'attesa.
 *
 * @param fd File descriptor da osservare
 * @param running_flag Flag che indica se continuare
 *          ad ascoltare (= 1), o terminare (= 0).
 */
void wait_until(int fd, const int *running_flag) {
    struct pollfd poll_info;
    poll_info.fd = fd;
    poll_info.events = POLLIN;
    poll_info.revents = 0;

    int poll_res;

    while ((poll_res = poll(&poll_info, 1, POLL_WAIT_TIMEOUT)) == 0 && *running_flag) {
        // Aspetta ancora
    }

    if (poll_res < 0) {
        // Errore
        perror("Errore poll in wait_until");
    }
}
