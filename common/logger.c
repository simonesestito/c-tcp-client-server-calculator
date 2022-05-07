#include "logger.h"
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <malloc.h>
#include <sys/file.h>

/**
 * Numero massimo di file di log possibili.
 * Se sono tutti non disponibili, non provare oltre.
 */
#define MAX_LOG_FILES 10

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
                const struct timestamp *start_time,
                const struct timestamp *end_time) {
    long start_microseconds = start_time->microseconds;
    long end_microseconds = end_time->microseconds;
    char start_time_str[TIMESTAMP_STRING_SIZE] = {};
    timestamp_to_string(start_time, start_time_str);

    log_message(client_info,
                "%s = %lf, da %s per %lu us\n",
                operation_line,
                result,
                start_time_str,
                end_microseconds - start_microseconds);
}

/**
 * Apri il file di log, se non è ancora stato aperto.
 * Usa la modalità di append.
 *
 * @param filename Nome del file di log di default, senza estensione .log
 * @param log_counter Indice del nome del file di log effettivamente usato,
 *          provando fino a MAX_LOG_FILES.
 *          Formato del file finale: filename (%d).log
 * @return File pointer al log
 */
FILE *_open_log_file(const char *original_filename, unsigned short *log_counter) {
    static FILE *log_file = NULL;

    if (original_filename == NULL || log_counter == NULL || log_file != NULL)
        return log_file;

    // Numero del file di log che sto provando
    *log_counter = 0;

    // Nome del file di log del numero corrispondente a file_counter
    size_t count_filename_len = strlen(original_filename) + 9;
    char count_filename[count_filename_len];
    strcpy(count_filename, original_filename);
    strcat(count_filename, ".log");

    while (log_file == NULL && *log_counter < MAX_LOG_FILES) {
        // Aggiorna il nome del file di log in base al numero del tentativo
        if (*log_counter > 0) {
            snprintf(count_filename, count_filename_len, "%s (%d).log", original_filename, *log_counter);
        }

        log_file = fopen(count_filename, "a");
        if (log_file == NULL) {
            // Errore nell'apertura del file
            perror("Errore nell'apertura del file di log");
        } else if (flock(fileno(log_file), LOCK_EX | LOCK_NB) == -1) {
            // Impossibile acquisire il lock su quel file, quale è la causa?
            // EWOULDBLOCK => c'è già un lock da un altro PROCESSO.
            if (errno != EWOULDBLOCK) {
                // Altro errore imprevisto
                perror("flock sul file di log");
            }

            // Errore, scarta questo file.
            fclose(log_file);
            log_file = NULL;
        }

        // Prova al prossimo file
        (*log_counter)++;
    }

    // Ultima iterazione lo manda oltre il reale numero del tentativo
    (*log_counter)--;

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
    unsigned short log_file_number = 0;
    FILE *log_file = _open_log_file(filename, &log_file_number);
    if (log_file == NULL)
        return -1;

    struct timestamp current_time;
    get_timestamp(&current_time);
    char time_str[TIME_STRING_SIZE];
    time_to_string((const struct tm *) &current_time, time_str);

    log_message(NULL, "===================================\n");
    log_message(NULL, "=========== Nuovo avvio ===========\n");
    log_message(NULL, "======= %s =======\n", time_str);
    log_message(NULL, "===================================\n");

    // Comunica all'utente su che file di log stiamo scrivendo
    if (log_file_number == 0)
        log_message(NULL, "File di log usato: %s.log\n", filename);
    else
        log_message(NULL, "File di log usato: %s (%d).log\n", filename, log_file_number);

    return 0;
}

/**
 * Apri il file di log, se non è ancora stato aperto.
 * Usa la modalità di append.
 *
 * @return File pointer al log
 */
FILE *open_log_file() {
    return _open_log_file(NULL, NULL);
}

/**
 * Termina tutte le operazioni di log, chiudi il file e libera la memoria
 */
void close_logging() {
    // Rimuovi tutti i log in memoria
    for (int i = 0; i < LOGS_ARRAY_SIZE; i++)
        free(logs_array[i]);

    // Chiudi il file di log e rimuovi il lock
    if (open_log_file() != NULL) {
        flock(fileno(open_log_file()), LOCK_UN);
        fclose(open_log_file());
    }
}
