#ifndef SERVER_REQUEST_WORKER_H
#define SERVER_REQUEST_WORKER_H

#include <netinet/in.h>

/**
 * Raccogli le informazioni del client
 */
struct sock_info {
    /**
     * File descriptor del socket col client
     */
    int socket_fd;

    /**
     * Informazioni aggiuntive sul socket
     */
    struct sockaddr_in client_info;
};

/**
 * Elabora la connessione / richiesta ricevuta dal client.
 *
 * Una volta ricevuta la connessione, è bene eseguire questa procedura
 * su un thread a sè.
 *
 * La struttura delle informazioni del client verrà liberata a fine esecuzione.
 *
 * @param client_info  Informazioni sulla connessione col client
 */
void elaborate_request(const struct sock_info *client_info);

#endif //SERVER_REQUEST_WORKER_H
