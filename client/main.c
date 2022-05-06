#include <stdlib.h>
#include <wchar.h>
#include <signal.h>
#include <errno.h>
#include "../common/calc_utils.h"
#include "../common/main_init.h"
#include "../common/logger.h"
#include "socket_utils.h"
#include "chart.h"

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

int get_user_input(operand_t *left_operand, operand_t *right_operand, char *operator);

void do_server_operations(FILE* socket_input, FILE* socket_output);

int main(int argc, const char **argv) {
    // Inizializza log, connessione, etc
    const char *ip;
    uint16_t port;
    if (main_init(argc, argv, NULL, connect_to_server, &ip, &port) != 0)
        return EXIT_FAILURE;

    // Gestisci i casi di SIGPIPE, che altrimenti di default terminano il programma
    handle_signal(SIGPIPE, handle_sigpipe);

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
            do_server_operations(socket_input, socket_output);
        }

        // Ripulisci la memoria e le risorse utilizzate
        fclose(socket_input);
        fclose(socket_output);
        free(chart_data);
        chart_data = NULL;
        chart_data_len = 0;
    }

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
 * Richiedi in input all'utente l'operazione da inviare al server
 *
 * @param left_operand Dove verrà memorizzato l'operando di sinistra
 * @param right_operand Dove verrà memorizzato l'operando di destra
 * @param operator Dove verrà memorizzato l'operatore
 * @return 0 se i dati sono stati ottenuti, -1 in caso non sia possibile neanche riprovare
*/
int get_user_input(operand_t *left_operand, operand_t *right_operand, char *operator) {
    int values_read; // Valori letti da scanf
    do {
        wprintf(L"\nPer uscire, premere CTRL+D\n");
        wprintf(L"Prossimo calcolo: ");
        fflush(stdout);

        values_read = scanf("%lf %c %lf", left_operand, operator, right_operand);

        if (values_read == EOF) {
            // Non sarà mai più possibile avere input.
            wprintf(L"Chiusura in corso...\n");
            return -1;
        } else if (values_read < 3) {
            // Input errato ma ancora è possibile riprovare
            wprintf(L"Input errato, riprovare.\n");

            // Svuota buffer scanf
            int ch;
            while (((ch = getchar()) != '\n') && (ch != EOF));
        }
    } while (values_read < 3);

    return 0;
}

/**
 * Esegui operazioni con il server.
 *
 * @param socket_input FILE* dove leggere dati
 * @param socket_output FILE* dove scrivere dati
 */
void do_server_operations(FILE* socket_input, FILE* socket_output) {
    // Variabili usate nella comunicazione
    operand_t left_operand, right_operand, result;
    char operator;
    unsigned long start_timestamp, end_timestamp;

    while (socket_fd > 0 && working) {
        // Leggi l'input utente
        if (get_user_input(&left_operand, &right_operand, &operator) == -1) {
            // Non sarà più possibile avere input utente. Termina.
            socket_fd = 0;
            working = 0;
            continue;
        }

        // Invia i dati al server
        fprintf(socket_output, "%c %lf %lf\n", operator, left_operand, right_operand);
        fflush(socket_output);

        // Leggi la risposta
        int server_read_values = fscanf(socket_input, "%lu %lu %lf", &start_timestamp, &end_timestamp, &result);
        if (server_read_values == 3) {
            // Risultato ricevuto correttamente
            // Aggiorna il grafico
            update_chart(end_timestamp - start_timestamp);

            wprintf(L"[%lu us] %lf %c %lf = %lf\n\n",
                    end_timestamp - start_timestamp,
                    left_operand,
                    operator,
                    right_operand,
                    result);
        } else if (errno == 0) {
            // C'è stato un EOF, la connessione è stata chiusa.
            log_message(NULL, "La connessione col server è stata chiusa.\n");
            // Esegui ri-connessione
            socket_fd = 0;
        } else {
            // La connessione non è chiusa ma c'è stato un errore nell'input
            log_errno(NULL, "Errore nella ricezione dei dati.\n");
        }
    }
}