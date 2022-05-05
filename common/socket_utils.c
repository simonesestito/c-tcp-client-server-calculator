#include "socket_utils.h"
#include <arpa/inet.h>
#include <errno.h>
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
        // Aspetta, ancora nessun dato e il server è in esecuzione
    }

    if (poll_res < 0) {
        // Errore
        perror("Errore poll in wait_until");
    }
}

/**
 * Elimina il \n o \r\n finali di una stringa ottenuta da una socket da getline.
 *
 * @param line Linea da cui rimuovere i caratteri
 * @param size Dimensione della linea, inclusi questi caratteri
 */
void strip_newline(char *line, ssize_t *size) {
    while (*size > 1 && (line[*size - 1] == '\n' || line[*size - 1] == '\r')) {
        line[*size - 1] = '\0';
        (*size)--;
    }
}
