#include <stdlib.h>
#include <wchar.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "../common/calc_utils.h"
#include "../common/main_init.h"
#include "../common/logger.h"
#include "socket_utils.h"
#include "chart.h"
#include "io_utils.h"

/**
 * Gestisci il SIGPIPE, notificando l'evento a tutti.
 */
void handle_sigpipe() {
    // Resetta il socket File descriptor cosi da far capire
    // a tutti che la connessione non è più valida.
    // Rimane ancora in esecuzione per riprovare la connessione.
    socket_fd = 0;
}

/**
 * Vettore dei tempi delle operazioni, da mostrare nel grafico
 */
unsigned int *chart_data = NULL;

/**
 * Spazio allocato da chart_data
 */
size_t chart_data_allocated = 1;

/**
 * Spazio utilizzato da chart_data (= indice della prossima locazione libera)
 */
size_t chart_data_len = 0;

void update_chart(unsigned int new_time);

void do_server_operations(FILE *socket_input, FILE *socket_output, operand_t *left_operand, operand_t *right_operand,
                          char *operator);

int main(int argc, const char **argv) {
    // Disattiva fdsan per fare dei test anche su Android
#if __has_include(<android/fdsan.h>)
#include <android/fdsan.h>
    android_fdsan_set_error_level(ANDROID_FDSAN_ERROR_LEVEL_DISABLED);
#endif

    // Inizializza log, connessione, etc
    const char *ip;
    uint16_t port;
    if (main_init(argc, argv, NULL, connect_to_server, &ip, &port) != 0)
        return EXIT_FAILURE;

    // Gestisci i casi di SIGPIPE, che altrimenti di default terminano il programma
    handle_signal(SIGPIPE, handle_sigpipe);

    // Variabili per l'operazione
    operand_t left_operand = 0, right_operand = 0;
    char operator = '\0';

    // Finché posso ancora interagire con l'utente...
    while (working && !feof(stdin)) {
        // Riconnettiti al server
        if (socket_fd <= 0 && (socket_fd = reconnect_exponential(ip, port)) == -1) {
            working = 0; // Indica la fase di terminazione del programma
            continue;
        }

        // Ripulisci lo schermo e inizializza l'area per il grafo
        plot_chart(chart_data, chart_data_len);

        // Apri il socket file descriptor come FILE pointer per usare funzioni
        // di libreria come fprintf e fscanf.

        FILE *socket_input = fdopen(socket_fd, "r");
        FILE *socket_output = fdopen(socket_fd, "w");

        if (socket_input == NULL || socket_output == NULL) {
            perror("fdopen");
        } else {
            // Finché ho una connessione al server valida...
            do_server_operations(socket_input, socket_output, &left_operand, &right_operand, &operator);
        }

        fclose(socket_input);
        fclose(socket_output);
    }

    // Ripulisci la memoria e le risorse utilizzate
    free(chart_data);
    chart_data = NULL;
    chart_data_len = 0;

    close_logging();
    // Se la socket è chiusa, dopo i vari tentativi, e l'utente NON ha dato CTRL+D,
    // considerala come un'esecuzione fallimentare.
    return socket_fd <= 0 && working && !feof(stdout) ? EXIT_FAILURE : EXIT_SUCCESS;
}

/**
 * Aggiorna il grafico mostrato all'utente sui tempi delle operazioni
 *
 * @param new_time Nuovo tempo impiegato
 */
void update_chart(unsigned int new_time) {
    if (chart_data == NULL) {
        chart_data = calloc(chart_data_allocated, sizeof(unsigned int));
    }

    if (chart_data_allocated == chart_data_len) {
        chart_data_allocated *= 2;
        chart_data = realloc(chart_data, sizeof(unsigned int) * chart_data_allocated);
    }

    chart_data[chart_data_len++] = new_time;

    plot_chart(chart_data, chart_data_len);
}

/**
 * Esegui operazioni con il server.
 *
 * @param socket_input FILE* dove leggere dati
 * @param socket_output FILE* dove scrivere dati
 * @param left_operand Scrive l'operando sinistro dell'ultimo input utente
 * @param right_operand Scrive l'operando destro dell'ultimo input utente
 * @param operator Scrive l'operatore dell'ultimo input utente
 */
void do_server_operations(FILE *socket_input, FILE *socket_output, operand_t *left_operand, operand_t *right_operand,
                          char *operator) {
    // Variabili necessarie nelle operazioni col server
    char raw_server_line[TIMESTAMP_STRING_SIZE * 3] = {};
    struct timestamp start_time, end_time;
    char start_time_str[TIMESTAMP_STRING_SIZE] = {};
    char end_time_str[TIMESTAMP_STRING_SIZE] = {};
    operand_t result;

    while (socket_fd > 0 && working) {
        // Leggi l'input utente, se non c'è già un input vecchio prima della ri-connessione
        if (*operator == '\0' && get_user_input(left_operand, right_operand, operator) == -1) {
            // Non sarà più possibile avere input utente. Termina.
            socket_fd = 0;
            working = 0;
        } else if (send_operation_to_server(socket_output, left_operand, right_operand, *operator) == 0 &&
                   recv_operation_from_server(socket_input, raw_server_line) == 0) {
            // Invio operazione e ricezione risposta dal server, entrambi avvenuti con successo.
            wprintf(L"\e[1;1H\e[2J>>>   %lf %c %lf   <<<\n", *left_operand, *operator, *right_operand);

            // Ancora dobbiamo interpretare la risposta (errore, dati, sconosciuto).
            if (parse_server_result(raw_server_line, &start_time, &end_time,
                                    &result, start_time_str, end_time_str) == 0) {
                // Tutte le operazioni si sono concluse con successo!
                // Calcola la differenza del tempo
                char diff_time_str[TIMESTAMP_STRING_SIZE] = {};
                timediff_to_string(&start_time, &end_time, diff_time_str);
                uint16_t diff_micros = timestamp_to_micros(&end_time) - timestamp_to_micros(&start_time);

                // Aggiorna il grafico
                update_chart(diff_micros);

                // Mostra il risultato
                wprintf(L"Risultato calcolato: %lf\n", result);
                wprintf(L"Ricezione richiesta: %s\n", start_time_str);
                wprintf(L"Fine elaborazione:   %s\n", end_time_str);
                wprintf(L"Tempo trascorso:     %s\n\n", diff_time_str);
            }

            // Visto che il server ci ha risposto in qualche modo,
            // non dobbiamo riprovare l'operazione più tardi.
            // Pulisci l'input utente.
            *operator = '\0';
            *left_operand = 0;
            *right_operand = 0;
        }
    }
}
