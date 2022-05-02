#include <locale.h>
#include <arpa/inet.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>

#define DEFAULT_PORT 12345
#define DEFAULT_HOST "127.0.0.1"
#define BACKLOG_SIZE 128

/**
 * Converti una stringa in un numero intero senza segno a 16 bit
 * @return Numero convertito, o 0 in caso di errore
 */
uint16_t str_to_uint16(const char *str) {
    int number = atoi(str);
    if (number < 0 || number > 65535)
        return 0;
    else
        return number;
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
 * Crea il socket per il server, esegui il bind, metti in ascolto.
 *
 * @param ip Indirizzo IP del server
 * @param port Porta del server
 * @return -1 in caso di errore, 0 se tutto con successo
 */
int bind_server(const char *ip, uint16_t port) {
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address)); // Azzera la struct
    server_address.sin_port = htons(port); // Imposta la porta
    server_address.sin_family = AF_INET; // Imposta IPv4

    // Prova ad impostare l'indirizzo IP
    if (inet_pton(AF_INET, ip, &(server_address.sin_addr)) != 1) {
        fprintf(stderr, "ERRORE: Indirizzo IP invalido\n");
        return -1;
    }

    // Crea la socket (unnamed)
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Errore nella creazione della socket");
        return -1;
    }

    // Esegui il bind
    if (bind(socket_fd, (const struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
        perror("Errore nel bind del socket");
        return -1;
    }

    // Metti in ascolto
    if (listen(socket_fd, BACKLOG_SIZE) == -1) {
        perror("Errore nella listen del socket");
        return -1;
    }

    return 0;
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

    if (bind_server(ip, port) == -1)
        return show_usage(argv[0], NULL);

    exit(EXIT_SUCCESS);
}