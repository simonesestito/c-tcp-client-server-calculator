#ifndef SERVER_LOGGER_H
#define SERVER_LOGGER_H

#include "request_worker.h"
#include "calc_utils.h"
#include <wchar.h>

/**
 * Dimensione richiesta del buffer per il prefisso del messaggio di log.
 */
#define LOG_PREFIX_SIZE 24

/**
 * Nome del file di log
 */
#define DEFAULT_LOG_FILENAME "server.log"

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
 * @param start_microseconds Tempo in microsecondi di inizio del calcolo
 * @param end_microseconds Tempo in microsecondi di fine del calcolo
 */
void log_result(const struct sock_info *client_info,
                const char *operation_line,
                operand_t result,
                uint64_t start_microseconds,
                uint64_t end_microseconds);

/**
 * Esegui il log di inizio di una nuova sessione del server.
 *
 * Utile dato che il log è in append, per capire di che avvio si sta parlando.
 * @return -1 in caso di errore, 0 se con successo.
 */
int log_new_start();

/**
 * Apri il file di log, se non è ancora stato aperto.
 * Usa la modalità di append.
 *
 * @return File pointer al log
 */
FILE* open_log_file();

#endif //SERVER_LOGGER_H
