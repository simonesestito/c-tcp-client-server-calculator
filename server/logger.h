#ifndef SERVER_LOGGER_H
#define SERVER_LOGGER_H

#include "request_worker.h"
#include <wchar.h>

/**
 * Dimensione richiesta del buffer per il prefisso del messaggio di log.
 */
#define LOG_PREFIX_SIZE 24

/**
 * Ottieni il prefisso del messaggio di log,
 * riportante informazioni sulla connessione.
 *
 * @param client_info Informazioni sulla connessione
 * @param buffer Dove memorizzare il prefisso. Grande almeno LOG_PREFIX_SIZE.
 */
void get_prefix(const struct sock_info *client_info, char *buffer);

/**
 * Esegui il log, inserendo anche le informazioni sul client.
 *
 * @param client_info Informazioni sul client
 * @param format Formato della stringa di log, nel formato printf
 * @param ... Argomenti della stringa di log
 */
void log_message(const struct sock_info *client_info, const wchar_t *restrict format, ...);

/**
 * Esegui il log di error number, inserendo anche le informazioni sul client.
 *
 * @param client_info Informazioni sul client che ha provocato l'errore
 * @param error_msg Messaggio di errore
 */
void log_errno(const struct sock_info *client_info, const char *error_msg);

/**
 * Esegui il log di un risultato ottenuto con successo.
 *
 * @param client_info Informazioni sul client che ha provocato l'errore
 * @param operation_line Linea di input dell'operazione effettuata
 * @param result Risultato dell'operazione
 * @param start Tempo di inizio del calcolo
 * @param end Tempo di fine del calcolo
 */
void log_result(const struct sock_info *client_info,
                const char *operation_line,
                operand_t result,
                const struct timespec *start,
                const struct timespec *end);

#endif //SERVER_LOGGER_H
