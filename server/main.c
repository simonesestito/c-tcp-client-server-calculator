#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include "socket_utils.h"
#include "request_worker.h"


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

int main(int argc, const char **argv) {
    // Dato che verranno usati wchar_t, il terminale deve essere
    // in modalità "wide charset" e serve impostare il locale.
    // Tutte le printf dovranno essere wprintf, altrimenti
    // il comportamento non è definito nelle specifiche del C
    setlocale(LC_CTYPE, "");

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

    int server_socket = bind_server(ip, port);
    if (server_socket == -1)
        return show_usage(argv[0], NULL);

    while (1) {
        // Accetta la prossima richiesta
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int client_socket = accept(server_socket, (struct sockaddr *) &client, &client_len);

        if (client_socket == -1) {
            // Errore nell'accettazione della richiesta
            perror("Accettazione nuova richiesta TCP");
        } else {
            // Gestisci la richiesta su un nuovo thread
            pthread_t request_thread;
            struct sock_info *socket_info = malloc(sizeof(struct sock_info));
            socket_info->socket_fd = client_socket;
            socket_info->client_info = client;
            if (pthread_create(&request_thread,
                               NULL,
                               (void *(*)(void *)) elaborate_request,
                               (void *) socket_info) != 0) {
                // Errore nella creazione del thread
                perror("Errore nella creazione del thread per la gestione della connessione TCP");
                close(client_socket);
            }
        }
    }
}