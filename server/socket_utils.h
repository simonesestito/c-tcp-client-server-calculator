#ifndef HW2_SOCKET_UTILS_H
#define HW2_SOCKET_UTILS_H

#include "../common/socket_utils.h"

/**
 * Crea il socket per il server, esegui il bind, metti in ascolto.
 *
 * Implementata solo nel programma SERVER.
 *
 * @param ip Indirizzo IP del server
 * @param port Porta del server
 * @return -1 in caso di errore, il file descriptor del server socket altrimenti
 */
int bind_server(const char *ip, uint16_t port);

#endif //HW2_SOCKET_UTILS_H
