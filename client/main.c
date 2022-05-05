#include <stdlib.h>
#include <wchar.h>
#include <signal.h>
#include <errno.h>
#include "../common/calc_utils.h"
#include "../common/socket_utils.h"
#include "../common/main_init.h"
#include "../common/logger.h"
#include "chart.h"

/**
 * Gestisci il SIGPIPE, notificando l'evento a tutti.
 */
void handle_sigpipe() {
    // Resetta il socket File descriptor cosi da far capire
    // a tutti che la connessione non è più valida.
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

int main(int argc, const char **argv) {
    // Inizializza
    if (main_init(argc, argv, "client.log", connect_to_server) != 0)
        return EXIT_FAILURE;

    FILE *socket_input = fdopen(socket_fd, "r");
    FILE *socket_output = fdopen(socket_fd, "w");

    signal(SIGPIPE, handle_sigpipe);

    operand_t left_operand, right_operand, result;
    char operator;
    unsigned long start_timestamp, end_timestamp;

    while (1 /* FIXME */) {
        wprintf(L"Prossimo calcolo: ");
        fflush(stdout);

        if (scanf("%lf %c %lf", &left_operand, &operator, &right_operand) < 3) {
            // Errore nell'input
            wprintf(L"Operazione errata\n");
            // FIXME: client, ripeti input
            break;
        }

        fprintf(socket_output, "%c %lf %lf\n", operator, left_operand, right_operand);
        fflush(socket_output);

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
            break; // FIXME, gestisci riconnessione
        } else {
            // La connessione non è chiusa ma c'è stato un errore nell'input
            log_errno(NULL, "Errore nella ricezione dei dati.\n");
        }
    }

    fclose(socket_input);
    fclose(socket_output);
    free(chart_data);

    close_logging();
    return EXIT_SUCCESS;
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