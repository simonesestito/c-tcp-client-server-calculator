#include "logger.h"
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>

/**
 * Ottieni il prefisso del messaggio di log,
 * riportante informazioni sulla connessione.
 *
 * @param client_info Informazioni sulla connessione
 * @param buffer Dove memorizzare il prefisso. Grande almeno LOG_PREFIX_SIZE.
 */
void get_prefix(const struct sock_info *client_info, char *buffer) {
    if (client_info == NULL) {
        strcpy(buffer, "[MAIN]");
    } else {
        // Ottieni i dati per la stringa del tipo [192.168.100.100:12345]
        char *ip = inet_ntoa(client_info->client_info.sin_addr);
        uint16_t port = htons(client_info->client_info.sin_port);
        snprintf(buffer, LOG_PREFIX_SIZE, "[%s:%d]", ip, port);
    }
}

/**
 * Esegui il log di error number, inserendo anche le informazioni sul client.
 *
 * @param client_info Informazioni sul client che ha provocato l'errore
 * @param error_msg Messaggio di errore
 */
void log_errno(const struct sock_info *client_info, const char *error_msg) {
    // Concatena messaggio di errore e descrizione di error number
    const char *errno_msg = strerror(errno);
    char full_msg[strlen(error_msg) + 3 + strlen(errno_msg)];
    strcpy(full_msg, error_msg);
    strcat(full_msg, errno_msg);
    log_message(client_info, L"%s\n", full_msg);
}

/**
 * Esegui il log, inserendo anche le informazioni sul client.
 *
 * @param client_info Informazioni sul client
 * @param format Formato della stringa di log, nel formato printf
 * @param ... Argomenti della stringa di log
 */
void log_message(const struct sock_info *client_info, const wchar_t *restrict format, ...) {
    // Ottieni prefisso di log
    char prefix[LOG_PREFIX_SIZE] = {};
    get_prefix(client_info, prefix);

    // Leggi i variadic parameters dai ..., da passare a printf
    va_list args;
    va_start(args, format);

    // Stampa sia su stdout che su file
    // Siccome possono accadere più scritture concorrenti
    // sia su stdout che sul file di log,
    // andiamo ad acquisire un lock.
    // Tutte le funzioni che scrivono su stdout o file di log
    // devono volontariamente rispettare questo lock.
    flockfile(stdout);
    wprintf(L"%s ", prefix);
    vwprintf(format, args);
    funlockfile(stdout);

    if (client_info != NULL) {
        // TODO: Su file
        // TODO: Lock file flockfile
        //  fwprintf(L"%s ", prefix);
        //  vfwprintf(format, args);
        // TODO: Unlock file funlockfile
    }

    // Dealloca gli argomenti
    va_end(args);
}

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
                const struct timespec *end) {
    // FIXME: Deve essere una vera data? Ora è il numero di microsecondi
    log_message(client_info,
                L"%s = %lf, da %u.%u a %u.%u -> %u microsecondi\n",
                operation_line,
                result,
                start->tv_sec,
                start->tv_nsec / 1000,
                end->tv_sec,
                end->tv_nsec / 1000,
                (end->tv_nsec - start->tv_nsec) / 1000);
}
