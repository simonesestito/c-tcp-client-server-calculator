#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <wchar.h>
#include "socket_utils.h"
#include "request_worker.h"

/**
 * Indica se il server è in funzione o deve terminare
 */
int working = 1;

/**
 * File descriptor della server socket
 */
int server_socket = 0;

/**
 * Gestisci i segnali di uscita (SIGTERM, SIGINT, SIGQUIT).
 */
void handle_exit() {
    working = 0;
    if (server_socket > 0)
        close(server_socket);
}

/**
 * Mostra il messaggio di utilizzo.
 * @param exec Nome del file eseguibile del programma corrente
 * @param error Messaggio di errore, aggiuntivo all'help
 * @return Exit code di fallimento, così è possibile scrivere il return più compatto nel main
 */
int show_usage(const char *exec, const char *error) {
    if (error != NULL)
        fprintf(stderr, "ERRORE: %s\n", error);

    fprintf(stderr, "Utilizzo: %s [PORTA] [IP]\n", exec);
    fprintf(stderr, "Di default, PORTA = %d, IP = %s\n", DEFAULT_PORT, DEFAULT_HOST);

    return EXIT_FAILURE;
}

/**
 * Gestisci una richiesta in arrivo, inviandola a un altro Thread,
 * o mostra i casi di errore che si sono presentati.
 *
 * @param client_socket File descriptor della socket col client
 * @param client Informazioni aggiuntive sulla socket
 */
void handle_request(int client_socket, const struct sockaddr_in *client);

int main(int argc, const char **argv) {
    // Dato che verranno usati wchar_t, il terminale deve essere
    // in modalità "wide charset" e serve impostare il locale.
    // Tutte le printf dovranno essere wprintf, altrimenti
    // il comportamento non è definito nelle specifiche del C
    setlocale(LC_CTYPE, "");

    // Gestisci i segnali di chiusura
    signal(SIGINT, handle_exit);
    signal(SIGTERM, handle_exit);
    signal(SIGQUIT, handle_exit);

    // Ottieni la porta scelta dall'utente
    uint16_t port = argc < 2 ? DEFAULT_PORT : str_to_uint16(argv[1]);
    if (port == 0)
        return show_usage(argv[0], "Numero di porta invalido");

    // Ottieni l'indirizzo IP scelto dall'utente
    const char *ip;
    if (argc < 3)
        ip = DEFAULT_HOST;
    else
        ip = argv[2];

    server_socket = bind_server(ip, port);
    if (server_socket == -1)
        return show_usage(argv[0], NULL);

    wprintf(L"Processo server (pid=%d) avviato sull'indirizzo IP %s, porta %d\n", getpid(), ip, port);

    while (working) {
        // Accetta la prossima richiesta
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int client_socket = accept(server_socket, (struct sockaddr *) &client, &client_len);
        handle_request(client_socket, &client);
    }

    return EXIT_SUCCESS;
}

void handle_request(int client_socket, const struct sockaddr_in *client) {
    if (!working) {
        wprintf(L"\n\nChiusura in corso...\n");
    } else if (client_socket == -1) {
        // Errore nell'accettazione della richiesta
        perror("Accettazione nuova richiesta TCP");
    } else {
        // Gestisci la richiesta su un nuovo thread
        pthread_t request_thread;
        struct sock_info *socket_info = malloc(sizeof(struct sock_info));
        FILE *socket_file = fdopen(client_socket, "r+");
        socket_info->socket_file = socket_file; // Vedasi doc di struct sock_info
        socket_info->client_info = *client;

        if (socket_file == NULL) {
            // Errore nell'apertura del socket file descriptor in r+
            perror("Errore nell'apertura del file descriptor della socket");
        } else if (pthread_create(&request_thread,
                           NULL,
                           (void *(*)(void *)) elaborate_request,
                           (void *) socket_info) != 0) {
            // Errore nella creazione del thread
            perror("Errore nella creazione del thread per la gestione della connessione TCP");
            fclose(socket_file);
        }
    }
}