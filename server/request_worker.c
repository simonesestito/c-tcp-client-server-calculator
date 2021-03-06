#include "request_worker.h"
#include "../common/logger.h"
#include "../common/main_init.h"
#include "live_status_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

int parse_client_line(const struct sock_info *client_info, char *line, char *operator, operand_t *left_operand,
                      operand_t *right_operand, operand_t *result);

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
        // Ottieni la riga dell'operazione, può essere interrotto con SIGINT al thread
        chars_read = getline(&line, &line_size, client_info->socket_file);
        if (chars_read < 0) break;

        // Rimuovi il \n o \r\n finale
        strip_newline(line, &chars_read);

        // Inizia a calcolare il tempo
        struct timestamp start_time, end_time;
        get_timestamp(&start_time);

        // Effettua il parsing della linea e calcola l'operazione
        char operator;
        operand_t left_operand, right_operand;
        operand_t result;

        if (parse_client_line(client_info, line, &operator, &left_operand, &right_operand, &result) == 0) {
            // Conteggia una nuova operazione nel live status
            add_client_operation(client_info);

            // Termina il conteggio del tempo
            get_timestamp(&end_time);

            // Tieni traccia nel log
            log_result(client_info, line, result, &start_time, &end_time);

            // Invia la risposta al client
            // [timestamp ricezione richiesta, timestamp invio risposta, risultato operazione]
            char start_time_str[TIMESTAMP_STRING_SIZE] = {};
            char end_time_str[TIMESTAMP_STRING_SIZE] = {};
            timestamp_to_string(&start_time, start_time_str);
            timestamp_to_string(&end_time, end_time_str);
            fprintf(client_info->socket_file, "%s %s %lf\n", start_time_str, end_time_str, result);
        }

        fflush(client_info->socket_file);
    } while (chars_read > 0 && errno == 0);

    if (errno != 0 && working) {
        log_errno(client_info, "Impossibile leggere la linea");
    }

    remove_client(client_info);
    free(line);
    fflush(client_info->socket_file);
    fclose(client_info->socket_file);
    free((struct sock_info *) client_info);
    pthread_detach(pthread_self());
}

/**
 * Esegui il parsing della stringa del client, gestendo gli errori e i calcoli
 *
 * @param client_info Informazioni sul client
 * @param line Linea ricevuta dal client
 * @param operator Operatore estratto dalla stringa
 * @param left_operand Operando sinistro estratto dalla stringa
 * @param right_operand Operando destro estratto dalla stringa
 * @param result Risultato dell'operazione
 * @return -1 in caso di errore, 0 altrimenti
 */
int parse_client_line(const struct sock_info *client_info, char *line, char *operator, operand_t *left_operand,
                      operand_t *right_operand, operand_t *result) {

    if (sscanf(line, "%c %lf %lf", operator, left_operand, right_operand) < 3) { // NOLINT(cert-err34-c)
        // Errore nella lettura
        log_message(client_info, "Errore nel parsing dell'operazione\n");
        errno = 0;
        // Segnala l'errore al client, inviando una linea col solo errore
        fprintf(client_info->socket_file, "%cErrore del client\n", SERVER_ERROR_MESSAGE_PREFIX);
        return -1;
    }

    // Esegui il calcolo
    *result = calculate_operation(*left_operand, *operator, *right_operand);
    if (errno == EINVAL) {
        log_errno(client_info, "Operazione sconosciuta");
        errno = 0;
        // Segnala l'errore al client, inviando una linea che inizia per -
        fprintf(client_info->socket_file, "%cOperazione sconosciuta\n", SERVER_ERROR_MESSAGE_PREFIX);
        return -1;
    }

    return 0;
}
