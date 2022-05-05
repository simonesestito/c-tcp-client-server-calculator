#include "logger.h"
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <malloc.h>

/**
 * Ultimi messaggi di log scritti nel file
 */
char *logs_array[LOGS_ARRAY_SIZE] = {};

/**
 * Indice da cui iniziare a leggere nel vettore circolare dei log
 */
int logs_index = 0;

/**
 * Ottieni il prefisso del messaggio di log,
 * riportante informazioni sulla connessione.
 *
 * @param client_info Informazioni sulla connessione
 * @param buffer Dove memorizzare il prefisso. Grande almeno LOG_PREFIX_SIZE.
 */
void get_prefix(const struct sock_info *client_info, char *buffer) {
    if (client_info == NULL) {
        strcpy(buffer, "[MAIN] ");
    } else {
        // Ottieni i dati per la stringa del tipo [192.168.100.100:12345]
        char *ip = inet_ntoa(client_info->client_info.sin_addr);
        uint16_t port = htons(client_info->client_info.sin_port);
        snprintf(buffer, LOG_PREFIX_SIZE, "[%s:%d] ", ip, port);
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
    log_message(client_info, "%s\n", full_msg);
}

/**
 * Esegui il log, inserendo anche le informazioni sul client.
 *
 * @param client_info Informazioni sul client
 * @param format Formato della stringa di log, nel formato printf
 * @param ... Argomenti della stringa di log
 */
void log_message(const struct sock_info *client_info, const char *restrict format, ...) {
    // Leggi i variadic parameters dai ..., da passare a printf
    va_list args;

    // Ottieni prefisso di log
    char *log_line = calloc(LOG_LINE_MAX_SIZE, sizeof(char));
    get_prefix(client_info, log_line);

    // Mantieni il messaggio di log anche in una stringa
    // per mostrarlo tra gli ultimi log nella schermata
    // della tabella in tempo reale
    va_start(args, format);
    vsnprintf(log_line + strlen(log_line), LOG_LINE_MAX_SIZE, format, args);
    va_end(args);

    // Conserva negli ultimi log in memoria per mostrarlo nella tabella
    flockfile(stdout);
    free(logs_array[logs_index]);
    logs_array[logs_index] = log_line;
    logs_index = (logs_index + 1) % LOGS_ARRAY_SIZE;
    funlockfile(stdout); // Accesso ai dati che riguardano l'output (come il log), sono in mutex

    // Stampa il messaggio nel file di log
    // Qui ci interessa ottenere il lock sul file solo tra i thread
    fwrite(log_line, sizeof(char), strlen(log_line), open_log_file());
    fflush(open_log_file());
    wprintf(L"%s", log_line);
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
                "%s = %lf, da %lu a %lu -> %lu microsecondi\n",
                operation_line,
                result,
                start_microseconds,
                end_microseconds,
                end_microseconds - start_microseconds);
}

/**
 * Apri il file di log, se non è ancora stato aperto.
 * Usa la modalità di append.
 *
 * @param filename Nome del file di log
 * @return File pointer al log
 */
FILE *_open_log_file(const char *filename) {
    static FILE *log_file = NULL;

    if (log_file == NULL) {
        log_file = fopen(filename, "a");
        // FIXME: il file di log deve essere lo stesso tra più esecuzioni del programma o può variare?
        // FIXME: ci possono essere istanze multiple del server?
        // FIXME: cerca il prossimo file libero e non bloccato (1), (2), (3), etc??
    }

    return log_file;
}

/**
 * Esegui il log di inizio di una nuova sessione del server.
 *
 * Utile dato che il log è in append, per capire di che avvio si sta parlando.
 *
 * @param filename Nome del file di log da usare
 * @return -1 in caso di errore, 0 se con successo.
 */
int log_new_start(const char *filename) {
    FILE *log_file = _open_log_file(filename);
    if (log_file == NULL) {
        perror("Errore nell'apertura del file di log.");
        return -1;
    }

    time_t current_time = time(NULL);
    struct tm current_date = *localtime(&current_time);

    fprintf(open_log_file(), "\n===================================\n");
    fprintf(open_log_file(), "====== Nuovo avvio del server =====\n");
    fprintf(open_log_file(), "======= %02d/%02d/%d %02d:%02d:%02d =======\n",
            current_date.tm_mday,
            current_date.tm_mon + 1,
            current_date.tm_year + 1900,
            current_date.tm_hour,
            current_date.tm_min,
            current_date.tm_sec);
    fprintf(open_log_file(), "===================================\n");
    fflush(open_log_file());
    return 0;
}

/**
 * Apri il file di log, se non è ancora stato aperto.
 * Usa la modalità di append.
 *
 * @return File pointer al log
 */
FILE *open_log_file() {
    return _open_log_file(NULL);
}
