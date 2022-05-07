#include "chart.h"
#include "../common/calc_utils.h"
#include <wchar.h>
#include <sys/ioctl.h>
#include <unistd.h>

/**
 * Mostra sul terminale un grafico con i dati forniti in input.
 * 
 * Utilizza wchar_t, quindi tutte le printf (in tutto il programma)
 * devono essere di tipo WIDE, altrimenti è undefined behaviour.
 *
 * @param original_data Dati da mostrare
 * @param data_len Dimensione dei dati
 */
void plot_chart(const unsigned int *original_data, size_t data_len) {
    // Ottieni la larghezza del terminale
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int max_columns = (w.ws_col / 2) - 1;

    // Limita i dati agli ultimi max_columns
    size_t normalized_data_len;
    unsigned int normalized_data[max_columns];
    limit_data(original_data, normalized_data, &normalized_data_len, max_columns, data_len);

    // Trova il dato minore e maggiore.
    // Il minimo verrà mostrato ad altezza 1
    unsigned int min_data = min(normalized_data, normalized_data_len);
    unsigned int max_data = max(normalized_data, normalized_data_len);

    // Normalizza i dati in un intervallo [1:10].
    for (size_t i = 0; i < normalized_data_len; i++) {
        normalized_data[i] =
                max_data == min_data ? 1 : ((normalized_data[i] - min_data) * 9) / (max_data - min_data) + 1;
    }

    wprintf(L"%lc\n", ARROW_UP);
    // Itera sulle righe (altezze) del grafico
    for (int curr_height = 10; curr_height >= 1; curr_height--) {
        wprintf(L"%lc", VERTICAL_BAR);
        // Itera sugli elementi dei dati
        for (int i = 0; i < normalized_data_len; i++) {
            draw_single_cell(normalized_data, i, curr_height);
        }
        wprintf(L"\n");
    }

    wprintf(L"%lc", CORNER_BOTTOM_LEFT);
    for (int i = 0; i < max_columns * 2; i++)
        wprintf(L"%lc", HORIZONTAL_BAR);
    wprintf(L">\n");
}

/**
 * Limita i dati dell'array originale nei max_columns ultimi elementi.
 *
 * @param original_data Dati originali
 * @param limited_data Dati limitati
 * @param limited_data_len Dimensione effettivamente popolata di limited_data
 * @param max_data_len Limite massimo ai dati da inserire in limited_data
 * @param data_len Dimensione dei dati originali
 */
void limit_data(const unsigned int *original_data, unsigned int *limited_data, size_t *limited_data_len,
                int max_data_len, size_t data_len) {
    *limited_data_len = max_data_len < data_len ? max_data_len : data_len;
    size_t data_offset = data_len - *limited_data_len;
    for (size_t i = 0; i < *limited_data_len; i++) {
        limited_data[i] = original_data[data_offset + i];
    }
}

/**
 * Disegna una singola cella, grande 1x2, nel grafico
 *
 * @param data Dati di origine del grafico
 * @param i Indice della riga della cella
 * @param curr_height Altezza corrente nel grafico
 */
void draw_single_cell(const unsigned int *data, int i, int curr_height) {
    // Mostra il carattere corrispondente alla cella del grafico.
    // Il minimo e massimo tra l'elemento corrente e quello precedente
    unsigned int previous_element = i == 0 ? 0 : data[i - 1];
    unsigned int min_previous = previous_element < data[i] ? previous_element : data[i];
    unsigned int max_previous = previous_element > data[i] ? previous_element : data[i];

    // Stampa la linea verticale per collegare
    // l'elemento del grafico precedente al corrente
    // e disegnare così una barra verticale
    wchar_t prev_bar_char;
    if (previous_element == curr_height && curr_height == data[i])
        prev_bar_char = HORIZONTAL_BAR; // Sono uguali come altezza
    else if (curr_height < max_previous && curr_height > min_previous)
        prev_bar_char = VERTICAL_BAR; // Mi trovo tra i due livelli (precedente e corrente)
    else if (curr_height == max_previous && max_previous == data[i])
        prev_bar_char = CORNER_TOP_LEFT; // Fine della linea ascendente (corrente > precedente)
    else if (curr_height == max_previous)
        prev_bar_char = CORNER_TOP_RIGHT; // Fine della linea discendente (corrente < precedente)
    else if (curr_height == min_previous && min_previous == data[i])
        prev_bar_char = CORNER_BOTTOM_LEFT; // Inizio della linea discendente (corrente < precedente)
    else if (curr_height == min_previous)
        prev_bar_char = CORNER_BOTTOM_RIGHT; // Inizio della linea ascendente (corrente > precedente)
    else
        prev_bar_char = ' '; // Questa cella deve restare vuota

    wprintf(L"%lc", prev_bar_char);

    // Stampa la linea superiore della barra dell'elemento corrente
    wprintf(L"%lc", data[i] == curr_height ? HORIZONTAL_BAR : L' ');
}

