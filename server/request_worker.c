#include "request_worker.h"
#include "logger.h"
#include "calc_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

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
        uint64_t start_microseconds = get_current_microseconds();

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
        uint64_t end_microseconds = get_current_microseconds();

        // Invia la risposta al client
        // [timestamp ricezione richiesta, timestamp invio risposta, risultato operazione]
        fprintf(client_info->socket_file, "%lu %lu %lf\n",
                start_microseconds, end_microseconds, result);

        // Tieni traccia nel log
        log_result(client_info, line, result, start_microseconds, end_microseconds);
    } while (chars_read > 0 && errno == 0);

    if (errno != 0) {
        log_errno(client_info, "Impossibile leggere la linea");
    }

    free(line);
    fclose(client_info->socket_file);
    free((struct sock_info *) client_info);
}
