#ifndef HW2_MAIN_INIT_H
#define HW2_MAIN_INIT_H

#include <stdint-gcc.h>

/**
 * File descriptor del socket principale (server o client)
 */
extern int socket_fd;

/**
 * Funzione che prende un IP e una porta, restituendo un file descriptor di una socket.
 */
typedef int (*socket_initializer_t)(const char *, uint16_t);

/**
 * Procedura di avvio condivisa sia da client che da server.
 *
 * @param argc Numero degli argomenti
 * @param argv Argomenti in input
 * @param socket_init Azione di inizializzazione della socket principale
 * @param ip Dove scrivere l'indirizzo IP ricevuto
 * @param port Dove scrivere la porta ricevuta
 * @return 0 se tutto ok, -1 in caso di errore.
 */
int main_init(int argc, const char **argv, const char *log_filename, socket_initializer_t socket_init, const char **ip,
              uint16_t *port);

/**
 * Mostra il messaggio di utilizzo.
 *
 * @param exec Nome del file eseguibile del programma corrente
 * @return Exit code di fallimento, così è possibile scrivere il return più compatto nel main
 */
int show_usage(const char *exec);

/**
 * Leggi i parametri della socket IP e porta da argv.
 * Se non sono disponibili argomenti a sufficienza,
 * termina con successo e lascia le variabili invariate.
 *
 * @param port Numero di porta
 * @param ip Indirizzo IP
 * @param argc Numero degli argomenti in input
 * @param argv Argomenti in input
 * @return -1 in caso di errore, 0 se tutto ok
 */
int read_argv_socket_params(uint16_t *port, const char **ip, int argc, const char **argv);

#endif //HW2_MAIN_INIT_H
