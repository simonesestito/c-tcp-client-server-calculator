#include "request_worker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

/**
 * Esegui il log di error number su stderr, inserendo anche le informazioni sul client.
 *
 * @param client_info Informazioni sul client che ha provocato l'errore
 * @param error_msg Messaggio di errore
 */
void perror_socket(const struct sock_info *client_info, const char *error_msg);

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
    ssize_t chars_read = getline(&line, &line_size, client_info->socket_file);

    if (chars_read > 0) {
        // TODO: elabora richiesta
        fputs(line, client_info->socket_file);
        fflush(client_info->socket_file);
    } else if (errno != 0) {
        perror_socket(client_info, "Impossibile leggere la linea");
    }

    fclose(client_info->socket_file);
    free((struct sock_info *) client_info);
}

void perror_socket(const struct sock_info *client_info, const char *error_msg) {
    // Ottieni l'indirizzo IP come stringa da sin_addr
    char *ip_address = inet_ntoa(client_info->client_info.sin_addr);
    uint16_t port = htons(client_info->client_info.sin_port);

    // Concatena le due stringhe
    size_t full_msg_size = 25 /* Prefisso massimo del messaggio di log */ + strlen(error_msg);
    char full_msg[full_msg_size];
    snprintf(full_msg, full_msg_size, "[%s:%d] %s", ip_address, port, error_msg);

    // Output su stderr
    perror(full_msg);
}
