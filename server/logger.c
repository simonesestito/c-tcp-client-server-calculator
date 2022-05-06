#include "../common/logger.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/**
 * Esegui il log, inserendo anche le informazioni sul client.
 *
 * @param client_info Informazioni sul client
 * @param format Formato della stringa di log, nel formato printf
 * @param ... Argomenti della stringa di log
 */
void log_message(const struct sock_info *client_info, const char *restrict format, ...) {
    // Leggi i variadic parameters dai ..., da passare per creare il messaggio di log
    va_list args;
    va_start(args, format);
    char *log_line = calloc(LOG_LINE_MAX_SIZE, sizeof(char));
    get_prefix(client_info, log_line);
    vsnprintf(log_line + strlen(log_line), LOG_LINE_MAX_SIZE, format, args);
    va_end(args);

    // Conserva negli ultimi log in memoria per mostrarlo nella tabella
    flockfile(stdout);
    free(logs_array[logs_index]);
    logs_array[logs_index] = log_line;
    logs_index = (logs_index + 1) % LOGS_ARRAY_SIZE;
    funlockfile(stdout); // Accesso ai dati che riguardano l'output (come il log), sono in mutex

    // Stampa il messaggio nel file di log
    fwrite(log_line, sizeof(char), strlen(log_line), open_log_file());
    fflush(open_log_file());
    wprintf(L"%s", log_line);
}