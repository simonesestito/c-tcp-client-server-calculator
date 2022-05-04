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
    strcat(full_msg, ": ");
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

    // Stampa sia su stdout che su file
    // Siccome possono accadere più scritture concorrenti
    // sia su stdout che sul file di log,
    // andiamo ad acquisire un lock.
    // Tutte le funzioni che scrivono su stdout o file di log
    // devono volontariamente rispettare questo lock.
    flockfile(stdout);
    wprintf(L"%s ", prefix);
    va_start(args, format);
    vwprintf(format, args);
    va_end(args);
    funlockfile(stdout);

    // Qui ci interessa ottenere il lock sul file solo tra i thread
    flockfile(open_log_file());
    fwprintf(open_log_file(), L"%s ", prefix);
    va_start(args, format);
    vfwprintf(open_log_file(), format, args);
    va_end(args);
    fflush(open_log_file());
    funlockfile(open_log_file());
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
                uint64_t start_microseconds,
                uint64_t end_microseconds) {
    log_message(client_info,
                L"%s = %lf, da %lu a %lu -> %lu microsecondi\n",
                operation_line,
                result,
                start_microseconds,
                end_microseconds,
                end_microseconds - start_microseconds);
}

/**
 * Esegui il log di inizio di una nuova sessione del server.
 *
 * Utile dato che il log è in append, per capire di che avvio si sta parlando.
 * @return -1 in caso di errore, 0 se con successo.
 */
int log_new_start() {
    FILE *log_file = open_log_file();
    if (log_file == NULL) {
        perror("Errore nell'apertura del file di log.");
        return -1;
    }

    time_t current_time = time(NULL);
    struct tm current_date = *localtime(&current_time);

    flockfile(open_log_file());
    fwprintf(open_log_file(), L"\n===================================\n");
    fwprintf(open_log_file(), L"====== Nuovo avvio del server =====\n");
    fwprintf(open_log_file(), L"======= %02d/%02d/%d %02d:%02d:%02d =======\n",
             current_date.tm_mday,
             current_date.tm_mon + 1,
             current_date.tm_year + 1900,
             current_date.tm_hour,
             current_date.tm_min,
             current_date.tm_sec);
    fwprintf(open_log_file(), L"===================================\n");
    fflush(open_log_file());
    funlockfile(open_log_file());
    return 0;
}

/**
 * Apri il file di log, se non è ancora stato aperto.
 * Usa la modalità di append.
 *
 * @return File pointer al log
 */
FILE *open_log_file() {
    static FILE *log_file = NULL;

    if (log_file == NULL) {
        log_file = fopen(DEFAULT_LOG_FILENAME, "a");
        // FIXME: il file di log deve essere lo stesso tra più esecuzioni del programma o può variare?
        // FIXME: ci possono essere istanze multiple del server?
        // FIXME: cerca il prossimo file libero e non bloccato (1), (2), (3), etc??
    }

    return log_file;
}
