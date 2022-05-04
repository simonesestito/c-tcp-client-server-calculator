#include "request_worker.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

/**
 * Elabora il calcolo richiesto dal client.
 *
 * Imposta errno in caso di errore.
 *
 * @param left Operando di sinistra
 * @param operator Operatore
 * @param right Operando di destra
 * @return Il risultato dell'operazione, oppure 0 in caso di errore impostando errno.
 */
double calculate_operation(operand_t left, char operator, operand_t right) { // TODO: Spostare altrove
    errno = 0;
    switch (operator) {
        case '+':
            return left + right;
        case '-':
            return left - right;
        case '*':
            return left * right;
        case '/':
            return left / right;
        default:
            errno = EINVAL;
            return 0;
    }
}

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
    char *line = NULL;
    size_t line_size = 0;
    ssize_t chars_read;

    do {
        // Ottieni la riga dell'operazione
        chars_read = getline(&line, &line_size, client_info->socket_file);
        if (chars_read < 0) break;

        // Rimuovi il \n finale
        line[chars_read - 1] = '\0';

        // Inizia a calcolare il tempo
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);

        // Effettua il parsing della linea e calcola l'operazione
        char operator;
        operand_t left_operand, right_operand;
        if (sscanf(line, "%c %lf %lf", &operator, &left_operand, &right_operand) < 3) {
            // Errore nella lettura
            log_message(client_info, L"%s\n", "Errore nel parsing dell'operazione");
            errno = 0;
            continue; // TODO: Che fare? Chiudere connessione?
        }

        operand_t result = calculate_operation(left_operand, operator, right_operand);
        if (errno == EINVAL) {
            log_errno(client_info, "Operazione sconosciuta");
            errno = 0;
            continue;  // TODO: Che fare? Chiudere connessione?
        }

        // Termina il conteggio del tempo
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);

        // Invia la risposta al client
        // TODO: risposta         long microseconds_elapsed = (end.tv_nsec - start.tv_nsec) / 1000;

        // Tieni traccia nel log
        log_result(client_info, line, result, &start, &end);
    } while (chars_read > 0 && errno == 0);

    if (errno != 0) {
        log_errno(client_info, "Impossibile leggere la linea");
    }

    free(line);
    fclose(client_info->socket_file);
    free((struct sock_info *) client_info);
}
