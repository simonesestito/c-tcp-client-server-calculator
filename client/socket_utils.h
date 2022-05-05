#ifndef HW2_SOCKET_UTILS_H
#define HW2_SOCKET_UTILS_H

#include "../common/socket_utils.h"

/**
 * Valore per il file descriptor della socket che, a livello del programma,
 * significa che la socket è definitivamente chiusa,
 * ma è contro la volontà dell'utente, che vorrebbe usare il programma.
 */
#define SOCKET_WILL_RETRY 0

/**
 * Valore per il file descriptor della socket che, a livello del programma,
 * significa che la socket è definitivamente chiusa,
 * e anche l'utente ha smesso di usare il programma.
 */
#define SOCKET_DEAD -1

/**
 * Connettiti al server.
 *
 * Implementata solo nel programma CLIENT.
 *
 * @param ip Indirizzo IP del server
 * @param port Porta del server
 * @return -1 in caso di errore, il file descriptor del server socket altrimenti
 */
int connect_to_server(const char *ip, uint16_t port);

/**
 * Riconnettiti al server, eseguendo un backoff esponenziale.
 *
 * Implementata solo nel programma CLIENT.
 *
 * @param ip Indirizzo IP del server
 * @param port Porta del server
 * @return -1 in caso di errore per più di 3 volte, il file descriptor del server socket altrimenti
 */
int reconnect_exponential(const char *ip, uint16_t port);

#endif //HW2_SOCKET_UTILS_H
