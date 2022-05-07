#include "io_utils.h"
#include "../common/main_init.h"
#include "../common/logger.h"
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/**
 * Richiedi in input all'utente l'operazione da inviare al server
 *
 * @param left_operand Dove verrà memorizzato l'operando di sinistra
 * @param right_operand Dove verrà memorizzato l'operando di destra
 * @param operator Dove verrà memorizzato l'operatore
 * @return 0 se i dati sono stati ottenuti, -1 in caso non sia possibile neanche riprovare
*/
int get_user_input(operand_t *left_operand, operand_t *right_operand, char *operator) {
    int values_read; // Valori letti da scanf
    do {
        wprintf(L"\nPer uscire, premere CTRL+D\n");
        wprintf(L"Prossimo calcolo: ");
        fflush(stdout);

        values_read = scanf("%lf %c %lf", left_operand, operator, right_operand);

        if (values_read == EOF) {
            // Non sarà mai più possibile avere input.
            wprintf(L"Chiusura in corso...\n");
            return -1;
        } else if (values_read < 3) {
            // Input errato ma ancora è possibile riprovare
            wprintf(L"Input errato, riprovare.\n");

            // Svuota buffer scanf
            int ch;
            while (((ch = getchar()) != '\n') && (ch != EOF));
        }
    } while (values_read < 3);

    return 0;
}

/**
 * Invia i dati al server, se c'è ancora la connessione disponibile.
 *
 * @param socket_output FILE* dove comunicare col server
 * @param left_operand Operando di sinistra nell'operazione
 * @param right_operand Operando di destra nell'operazione
 * @param operator Operatore del calcolo
 * @return -1 in caso di errore, 0 altrimenti
 */
int send_operation_to_server(FILE *socket_output, const operand_t *left_operand, const operand_t *right_operand,
                             const char operator) {
    fprintf(socket_output, "%c %lf %lf\n", operator, *left_operand, *right_operand);
    fflush(socket_output);

    if (socket_fd == 0) {
        // C'è stata una SIGPIPE.
        log_message(NULL, "La connessione col server è stata chiusa (SIGPIPE in invio).\n");
        return -1;
    }

    return 0;
}

/**
 * Ricevi il risultato dell'operazione dal server, ancora in raw, senza parsing.
 *
 * @param socket_input FILE* da cui ricevere i dati dal server
 * @param raw_server_line Stringa dove scrivere la risposta raw del server
 * @return -1 in caso di errore, 0 altrimenti
 */
int recv_operation_from_server(FILE *socket_input, char *raw_server_line) {
    char *result = fgets(raw_server_line, TIMESTAMP_STRING_SIZE * 3, socket_input);

    if (result != NULL) {
        return 0; // Tutto ok
    }

    if (ferror(socket_input)) {
        log_errno(NULL, "recv_operation_from_server");
        clearerr(socket_input);
    } else if (feof(socket_input)) {
        // C'è stato un EOF, la connessione è stata chiusa.
        log_message(NULL, "La connessione col server è stata chiusa.\n");
        // Esegui ri-connessione
        socket_fd = 0;
    }

    return -1;
}

/**
 * Esegui il parsing della linea restituita dal server, gestendo gli errori di ogni parte.
 *
 * @param raw_server_line Linea raw ricevuta dal server
 * @param start_time Tempo di inizio calcolo
 * @param end_time Tempo di fine calcolo
 * @param result Risultato dell'operazione
 * @param start_time_str Stringa del tempo di inizio calcolo
 * @param end_time_str Stringa del tempo di fine calcolo
 * @return -1 in caso di errore, 0 altrimenti
 */
int parse_server_result(const char *raw_server_line, struct timestamp *start_time, struct timestamp *end_time,
                        operand_t *result, char *start_time_str, char *end_time_str) {
    if (raw_server_line[0] == SERVER_ERROR_MESSAGE_PREFIX) {
        // Questo messaggio rappresenta un errore.
        wprintf(L"[ERRORE] %s\n", raw_server_line + 1);
        return -1;
    }

    // Ottieni le parti della linea
    strncpy(start_time_str, raw_server_line, TIMESTAMP_STRING_SIZE);
    start_time_str[TIMESTAMP_STRING_SIZE - 1] = '\0';
    strncpy(end_time_str, raw_server_line + TIMESTAMP_STRING_SIZE, TIMESTAMP_STRING_SIZE);
    end_time_str[TIMESTAMP_STRING_SIZE - 1] = '\0';
    char *result_end_str = NULL;
    *result = strtod(raw_server_line + 2 * TIMESTAMP_STRING_SIZE, &result_end_str);

    if (*result_end_str != '\n') {
        // Errore nel parsing del risultato
        if (*result == 0 && errno != 0)
            log_errno(NULL, "Errore nel parsing del risultato");
        else
            log_message(NULL, "Errore nel parsing del risultato\n");
        return -1;
    }

    // Esegui il parsing dei tempi
    if (string_to_timestamp(start_time, start_time_str) == -1) {
        // Errore nel parsing del tempo d'inizio
        log_message(NULL, "Errore nel parsing del tempo d'inizio: %s\n", start_time_str);
        return -1;
    }

    if (string_to_timestamp(end_time, end_time_str) == -1) {
        // Errore nel parsing del tempo di fine
        log_message(NULL, "Errore nel parsing del tempo di fine: %s\n", end_time_str);
        return -1;
    }

    return 0;
}


