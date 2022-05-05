#include <stdlib.h>
#include <wchar.h>
#include <unistd.h>
#include "../common/calc_utils.h"
#include "../common/socket_utils.h"
#include "../common/main_init.h"
#include "../common/logger.h"
#include "chart.h"

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

    operand_t left_operand, right_operand, result;
    char operator;
    unsigned long start_timestamp, end_timestamp;

    while (1 /* FIXME */) {
        wprintf(L"Prossimo calcolo: ");
        fflush(stdout);

        if (scanf("%lf %c %lf", &left_operand, &operator, &right_operand) < 3) {
            // Errore nell'input
            wprintf(L"Operazione errata\n");
            // FIXME
            break;
        }


        fprintf(socket_output, "%c %lf %lf\n", operator, left_operand, right_operand);
        fflush(socket_output);

        if (fscanf(socket_input, "%lu %lu %lf", &start_timestamp, &end_timestamp, &result) < 3) {
            perror("fscanf recv operation");
        } else {
            // Aggiorna il grafico
            update_chart(end_timestamp - start_timestamp);

            wprintf(L"[%lu us] %lf %c %lf = %lf\n\n",
                    end_timestamp - start_timestamp,
                    left_operand,
                    operator,
                    right_operand,
                    result);
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