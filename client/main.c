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

void do_server_operations(FILE *socket_input, FILE *socket_output, operand_t *left_operand, operand_t *right_operand,
                          char *operator);

int main(int argc, const char **argv) {
    // Disattiva fdsan per fare dei test su Android
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
 * @param left_operand Scrive l'operando sinistro dell'ultimo input utente
 * @param right_operand Scrive l'operando destro dell'ultimo input utente
 * @param operator Scrive l'operatore dell'ultimo input utente
 */
void do_server_operations(FILE *socket_input, FILE *socket_output, operand_t *left_operand, operand_t *right_operand,
                          char *operator) {
    // Variabili per la risposta
    operand_t result;
    char start_time_str[TIMESTAMP_STRING_SIZE] = {};
    char end_time_str[TIMESTAMP_STRING_SIZE] = {};

    while (socket_fd > 0 && working) {
        // Leggi l'input utente, se non c'è già un input vecchio prima della ri-connessione
        if (*operator == '\0' && get_user_input(left_operand, right_operand, operator) == -1) {
            // Non sarà più possibile avere input utente. Termina.
            socket_fd = 0;
            working = 0;
            continue;
        }

        // Invia i dati al server, se c'è ancora la connessione disponibile
        fprintf(socket_output, "%c %lf %lf\n", *operator, *left_operand, *right_operand);
        fflush(socket_output);

        if (socket_fd == 0) {
            // C'è stata una SIGPIPE.
            wprintf(L"La connessione col server è stata chiusa (SIGPIPE in invio).\n");
            continue;
        }

        // Leggi la risposta
        char server_line[TIMESTAMP_STRING_SIZE*3] = {};
        fgets(server_line, TIMESTAMP_STRING_SIZE*3, socket_input);

        if (server_line[0] == '-') {
            // Caso di errore.
            // Secondo il protocollo, viene inviato l'errore dopo il prefisso.
            wprintf(L"[ERRORE] %lf %c %lf: %s\n", *left_operand, *operator, *right_operand, server_line + 1);
        } else if (1) {
            // TODO Funzione per il parsing fatto bene, su altro file
            // Split della linea
            strncpy(start_time_str, server_line, TIMESTAMP_STRING_SIZE - 1);
            start_time_str[TIMESTAMP_STRING_SIZE-1] = 0;
            strncpy(end_time_str, server_line + TIMESTAMP_STRING_SIZE, TIMESTAMP_STRING_SIZE - 1);
            end_time_str[TIMESTAMP_STRING_SIZE-1] = 0;

            // Esegui il parsing dei tempi
            struct timestamp start_time, end_time;
            string_to_timestamp(&start_time, start_time_str);
            string_to_timestamp(&end_time, end_time_str);
            long start_micros = start_time.microseconds;
            long end_micros = end_time.microseconds;
            char diff_str[TIMEDIFF_STRING_SIZE] = {};
            timediff_to_string(&start_time, &end_time, diff_str);

            // Parsing del risultato
            result = strtod(server_line + 2*TIMESTAMP_STRING_SIZE, NULL);

            // Risultato ricevuto correttamente, aggiorna il grafico
            update_chart(end_micros - start_micros);

            wprintf(L">>>   %lf %c %lf = %lf  <<<\n", *left_operand, *operator, *right_operand, result);
            wprintf(L"Ricezione richiesta: %s\n", start_time_str);
            wprintf(L"Fine elaborazione:   %s\n", end_time_str);
            wprintf(L"Tempo trascorso:     %s\n", diff_str);
        } else if (socket_fd == 0 || errno == 0) {
            // C'è stato un EOF, la connessione è stata chiusa.
            log_message(NULL, "La connessione col server è stata chiusa.\n");
            // Esegui ri-connessione
            socket_fd = 0;
        } else {
            // La connessione non è chiusa ma c'è stato un errore nell'input
            // FIXME Da gestire funzione a se.
            log_errno(NULL, "Errore nella ricezione dei dati.\n");
        }

        if (!feof(socket_input)) {
            // Pulisci l'input utente ora che è concluso.
            // Tenere l'input serve in caso di errore di connessione,
            // quindi per riprovare più tardi.
            *operator = '\0';
            *left_operand = 0;
            *right_operand = 0;
        }
    }
}
