#include "main_init.h"
#include "logger.h"
#include <locale.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * File descriptor del socket principale (server o client)
 */
int socket_fd = 0;

/**
 * Gestisci i segnali di uscita (SIGTERM, SIGINT, SIGQUIT).
 */
void _handle_exit() {
    if (socket_fd > 0) {
        close(socket_fd);
        socket_fd = 0;
    }
}

/**
 * Procedura di avvio condivisa sia da client che da server.
 *
 * @param argc Numero degli argomenti
 * @param argv Argomenti in input
 * @param socket_init Azione di inizializzazione della socket principale
 * @return 0 se tutto ok, -1 in caso di errore.
 */
int main_init(int argc, const char **argv, const char *log_filename, socket_initializer_t socket_init) {
    // Dato che verranno usati wchar_t, il terminale deve essere
    // in modalità "wide charset" e serve impostare il locale.
    // Tutte le printf dovranno essere wprintf, altrimenti
    // il comportamento non è definito nelle specifiche del C
    setlocale(LC_CTYPE, "");

    // Gestisci i segnali di chiusura
    signal(SIGINT, _handle_exit);
    signal(SIGTERM, _handle_exit);
    signal(SIGQUIT, _handle_exit);

    // Mostra il messaggio di avvio nel log
    if (log_new_start(log_filename) == -1)
        return EXIT_FAILURE;

    // Ottieni la porta e l'indirizzo scelti dall'utente
    uint16_t port = DEFAULT_PORT;
    const char *ip = DEFAULT_HOST;
    if (read_argv_socket_params(&port, &ip, argc, argv) == -1) {
        close_logging();
        return show_usage(argv[0]);
    }

    // Avvia la socket principale
    socket_fd = socket_init(ip, port);
    if (socket_fd == -1) {
        close_logging();
        return show_usage(argv[0]);
    }

    log_message(NULL,
                "Processo %s (pid=%d) avviato con indirizzo IP %s e porta %d\n",
                argv[0], getpid(), ip, port);

    return 0;
}

/**
 * Leggi i parametri della socket IP e porta da argv.
 * Se non sono disponibili argomenti a sufficienza,
 * termina con successo e lascia le variabili invariate.
 *
 * @param port Numero di porta
 * @param ip Indirizzo IP
 * @param argc Numero degli argomenti in input
 * @param argv Argomenti in input
 * @return -1 in caso di errore, 0 se tutto ok
 */
int read_argv_socket_params(uint16_t *port, const char **ip, int argc, const char **argv) {
    if (argc < 2)
        return 0;

    uint16_t input_port = str_to_uint16(argv[1]);
    if (input_port == 0) {
        // Errore di porta
        log_message(NULL, "Numero di porta scelto invalido");
        return -1;
    }
    *port = input_port;

    if (argc < 3)
        return 0;

    *ip = argv[2];
    return 0;
}

/**
 * Mostra il messaggio di utilizzo.
 *
 * @param exec Nome del file eseguibile del programma corrente
 * @return Exit code di fallimento, così è possibile scrivere il return più compatto nel main
 */
int show_usage(const char *exec) {
    fprintf(stderr, "Utilizzo: %s [PORTA] [IP]\n", exec);
    fprintf(stderr, "Di default, PORTA = %d, IP = %s\n", DEFAULT_PORT, DEFAULT_HOST);
    return EXIT_FAILURE;
}