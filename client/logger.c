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
    // Leggi i variadic parameters dai ..., da passare per creare
    // il messaggio di log da stampare su stderr
    va_list args;
    va_start(args, format);
    char *log_prefix = calloc(LOG_PREFIX_SIZE, sizeof(char));
    get_prefix(client_info, log_prefix);
    fprintf(stderr, "%s", log_prefix);
    vfprintf(stderr, format, args);
    va_end(args);
}