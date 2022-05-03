#include "request_worker.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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
void elaborate_request(const struct sock_info *client_info) {
    const char *str = "Hello, World!\n";
    write(client_info->socket_fd, str, strlen(str));

    close(client_info->socket_fd);
    free((struct sock_info *) client_info);
}
