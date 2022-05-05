#include "chart.h"
#include <wchar.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <strings.h>
#include <stdio.h>

/**
 * Simboli Unicode per disegnare il grafico
 */
#define ARROW_UP ((wchar_t) 0x25B2)
#define HORIZONTAL_BAR ((wchar_t) 0x2500)
#define VERTICAL_BAR ((wchar_t) 0x2502)
#define CORNER_BOTTOM_LEFT ((wchar_t) 0x2514)
#define CORNER_BOTTOM_RIGHT ((wchar_t) 0x2518)
#define CORNER_TOP_LEFT ((wchar_t) 0x250C)
#define CORNER_TOP_RIGHT ((wchar_t) 0x2510)

/**
 * Trova il numero minimo dall'array, non vuoto
 * @param data Array in cui trovare il minimo
 * @param data_len Dimensione dell'array
 * @return Il minimo numero trovato
 */
unsigned int min(const unsigned int *data, size_t data_len) {
    unsigned int min_number = data[0];
    for (size_t i = 1; i < data_len; i++) {
        if (data[i] < min_number)
            min_number = data[i];
    }
    return min_number;
}

/**
 * Trova il numero massimo dall'array, non vuoto
 * @param data Array in cui trovare il minimo
 * @param data_len Dimensione dell'array
 * @return Il massimo numero trovato
 */
unsigned int max(const unsigned int *data, size_t data_len) {
    unsigned long max_number = data[0];
    for (int i = 1; i < data_len; i++) {
        if (data[i] > max_number)
            max_number = data[i];
    }
    return max_number;
}

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
    if (data_len == 0)
        return;

    // Pulisci schermo
    wprintf(L"\e[1;1H\e[2J");

    // Ottieni la larghezza del terminale
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int max_columns = (w.ws_col / 2) - 1;

    // Limita i dati agli ultimi max_columns
    size_t normalized_data_len = max_columns < data_len ? max_columns : data_len;
    unsigned int normalized_data[normalized_data_len];
    size_t data_offset = data_len - normalized_data_len;
    fprintf(stderr, "NORM: %zu\n", normalized_data_len);
    fprintf(stderr, "DLEN: %zu\n", data_len);
    fprintf(stderr, "MCOL: %d\n", max_columns);
    for (size_t i = 0; i < normalized_data_len; i++) {
        normalized_data[i] = original_data[data_offset + i];
    }

    // Trova il dato minore e maggiore.
    // Il minimo verrà mostrato ad altezza 1
    unsigned int min_data = min(normalized_data, normalized_data_len);
    unsigned int max_data = max(normalized_data, normalized_data_len);

    // Normalizza i dati in un intervallo [1:10].
    for (size_t i = 0; i < normalized_data_len; i++) {
        normalized_data[i] = max_data == min_data
                             ? 1
                             : ((normalized_data[i] - min_data) * 9) / (max_data - min_data) + 1;
    }

    wprintf(L"%lc\n", ARROW_UP);
    // Itera sulle righe (altezze) del grafico
    for (unsigned long curr_height = 10; curr_height >= 1; curr_height--) {
        wprintf(L"%lc", VERTICAL_BAR);
        // Itera sugli elementi dei dati
        for (size_t i = 0; i < normalized_data_len; i++) {
            // Mostra il carattere corrispondente alla cella del grafico.
            // Il minimo e massimo tra l'elemento corrente e quello precedente
            unsigned int previous_element = i == 0 ? 0 : normalized_data[i - 1];
            unsigned int min_previous = previous_element < normalized_data[i] ? previous_element : normalized_data[i];
            unsigned int max_previous = previous_element > normalized_data[i] ? previous_element : normalized_data[i];

            // Stampa la linea verticale per collegare
            // l'elemento del grafico precedente al corrente
            // e disegnare così una barra verticale
            wchar_t prev_bar_char;
            if (previous_element == curr_height && curr_height == normalized_data[i])
                prev_bar_char = HORIZONTAL_BAR; // Sono uguali come altezza
            else if (curr_height < max_previous && curr_height > min_previous)
                prev_bar_char = VERTICAL_BAR; // Mi trovo tra i due livelli (precedente e corrente)
            else if (curr_height == max_previous && max_previous == normalized_data[i])
                prev_bar_char = CORNER_TOP_LEFT; // Fine della linea ascendente (corrente > precedente)
            else if (curr_height == max_previous)
                prev_bar_char = CORNER_TOP_RIGHT; // Fine della linea discendente (corrente < precedente)
            else if (curr_height == min_previous && min_previous == normalized_data[i])
                prev_bar_char = CORNER_BOTTOM_LEFT; // Inizio della linea discendente (corrente < precedente)
            else if (curr_height == min_previous)
                prev_bar_char = CORNER_BOTTOM_RIGHT; // Inizio della linea ascendente (corrente > precedente)
            else
                prev_bar_char = ' '; // Questa cella deve restare vuota

            wprintf(L"%lc", prev_bar_char);

            // Stampa la linea superiore della barra dell'elemento corrente
            wprintf(L"%lc", normalized_data[i] == curr_height ? HORIZONTAL_BAR : L' ');
        }
        wprintf(L"\n");
    }

    wprintf(L"%lc", CORNER_BOTTOM_LEFT);
    for (int i = 0; i < max_columns*2; i++)
        wprintf(L"%lc", HORIZONTAL_BAR);
    wprintf(L">\n");
}
