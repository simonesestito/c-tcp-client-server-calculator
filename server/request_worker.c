#include "request_worker.h"
#include "../common/logger.h"
#include "../common/main_init.h"
#include "live_status_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

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

    // Mostra il nuovo client nella tabella di stato
    register_client(client_info, pthread_self());

    do {
        // Ottieni la riga dell'operazione,
        // aspettando dati e vedendo se serve interrompere
        wait_until(fileno(client_info->socket_file), &socket_fd);
        if (socket_fd <= 0) break;

        chars_read = getline(&line, &line_size, client_info->socket_file);
        if (chars_read < 0) break;

        // Rimuovi il \n o \r\n finale
        strip_newline(line, &chars_read);

        // Inizia a calcolare il tempo
        uint64_t start_microseconds = get_current_microseconds();

        // Effettua il parsing della linea e calcola l'operazione
        char operator;
        operand_t left_operand, right_operand;
        if (sscanf(line, "%c %lf %lf", &operator, &left_operand, &right_operand) < 3) {
            // Errore nella lettura
            log_message(client_info, "Errore nel parsing dell'operazione\n");
            errno = 0;
            continue; // TODO: Che fare? Chiudere connessione? break
        }

        operand_t result = calculate_operation(left_operand, operator, right_operand);
        if (errno == EINVAL) {
            log_errno(client_info, "Operazione sconosciuta");
            errno = 0;
            continue;  // TODO: Che fare? Chiudere connessione? break
        }

        // Conteggia una nuova operazione nel live status
        add_client_operation(client_info);

        // Termina il conteggio del tempo
        uint64_t end_microseconds = get_current_microseconds();

        // Invia la risposta al client
        // [timestamp ricezione richiesta, timestamp invio risposta, risultato operazione]
        fprintf(client_info->socket_file, "%lu %lu %lf\n",
                start_microseconds, end_microseconds, result);
        fflush(client_info->socket_file);

        // Tieni traccia nel log
        log_result(client_info, line, result, start_microseconds, end_microseconds);
    } while (chars_read > 0 && errno == 0);

    if (errno != 0) {
        log_errno(client_info, "Impossibile leggere la linea");
    }

    remove_client(client_info);
    free(line);
    fflush(client_info->socket_file);
    fclose(client_info->socket_file);
    free((struct sock_info *) client_info);
    pthread_detach(pthread_self());
}
