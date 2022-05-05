#ifndef SERVER_SOCKET_UTILS_H
#define SERVER_SOCKET_UTILS_H

#include <netinet/in.h>
#include <stdio.h>

#define DEFAULT_PORT 12345
#define DEFAULT_HOST "127.0.0.1"
#define BACKLOG_SIZE 128

/**
 * Tempo di attesa per poll() e quindi wait_until()
 */
#define POLL_WAIT_TIMEOUT 3000

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
void wait_until(int fd, const int *running_flag);

/**
 * Elimina il \n o \r\n finali di una stringa ottenuta da una socket da getline.
 * @param line Linea da cui rimuovere i caratteri
 * @param size Dimensione della linea, inclusi questi caratteri
 */
void strip_newline(char *line, ssize_t *size);

#endif //SERVER_SOCKET_UTILS_H
