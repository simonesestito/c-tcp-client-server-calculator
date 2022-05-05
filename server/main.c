#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <pthread.h>
#include <wchar.h>
#include "../common/socket_utils.h"
#include "request_worker.h"
#include "../common/logger.h"
#include "../common/main_init.h"
#include "live_status_table.h"

/**
 * Gestisci una richiesta in arrivo, inviandola a un altro Thread,
 * o mostra i casi di errore che si sono presentati.
 *
 * @param client_socket File descriptor della socket col client
 * @param client Informazioni aggiuntive sulla socket
 */
void handle_request(int client_socket, const struct sockaddr_in *client);

int main(int argc, const char **argv) {
    // Inizializza
    if (main_init(argc, argv, "server.log", bind_server) != 0)
        return EXIT_FAILURE;

    // Mostra lo stato in live su stdout
    init_status_table();

    while (socket_fd) {
        // Accetta la prossima richiesta
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int client_socket = accept(socket_fd, (struct sockaddr *) &client, &client_len);
        handle_request(client_socket, &client);
    }

    stop_status_table();
    close_logging();

    return EXIT_SUCCESS;
}

void handle_request(int client_socket, const struct sockaddr_in *client) {
    if (socket_fd <= 0) {
        return; // Non fare nulla, il server è in fase di spegnimento
    } else if (client_socket == -1) {
        // Errore nell'accettazione della richiesta
        log_errno(NULL, "Accettazione nuova richiesta TCP");
    } else {
        // Gestisci la richiesta su un nuovo thread
        pthread_t request_thread;
        struct sock_info *socket_info = malloc(sizeof(struct sock_info));
        FILE *socket_file = fdopen(client_socket, "r+");
        socket_info->socket_file = socket_file; // Vedasi doc di struct sock_info
        socket_info->client_info = *client;

        if (socket_file == NULL) {
            // Errore nell'apertura del socket file descriptor in r+
            log_errno(NULL, "Errore nell'apertura del file descriptor della socket");
        } else if (pthread_create(&request_thread,
                                  NULL,
                                  (void *(*)(void *)) elaborate_request,
                                  (void *) socket_info) != 0) {
            // Errore nella creazione del thread
            log_errno(socket_info, "Errore nella creazione del thread per la gestione della connessione TCP");
            fclose(socket_file);
        }
    }
}