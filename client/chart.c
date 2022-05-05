#include "chart.h"
#include <wchar.h>
#include <sys/ioctl.h>
#include <unistd.h>

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
unsigned int max(const unsigned int *data, size_t data_len)  {
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
 * @param data Dati da mostrare
 * @param data_len Dimensione dei dati
 */
void plot_chart(unsigned int *data, size_t data_len) {
    if (data_len == 0)
        return;

    // Ottieni la larghezza del terminale
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_width = w.ws_col;
    
    // Trova il dato minore e maggiore.
    // Il minimo verrà mostrato ad altezza 1
    unsigned int min_data = min(data, data_len);
    unsigned int max_data = max(data, data_len);
    
    wprintf(L"%lc\n", ARROW_UP);
    // Itera sulle righe (altezze) del grafico
    for (unsigned long curr_height = max_data; curr_height >= min_data; curr_height--) {
        wprintf(L"%lc", VERTICAL_BAR);
        // Itera sugli elementi dei dati
        for (int i = 0; i < data_len; i++) {
            // Mostra il carattere corrispondente alla cella del grafico

            unsigned int previous_element = i == 0 ? 0 : data[i-1];
            
            // Il minimo e massimo tra l'elemento corrente e quello precedente
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
            else if (curr_height == max_previous && max_previous == previous_element)
                prev_bar_char = CORNER_TOP_RIGHT; // Fine della linea discendente (corrente < precedente)
            else if (curr_height == min_previous && min_previous == data[i])
                prev_bar_char = CORNER_BOTTOM_LEFT; // Inizio della linea discendente (corrente < precedente)
            else if (curr_height == min_previous && min_previous == previous_element)
                prev_bar_char = CORNER_BOTTOM_RIGHT; // Inizio della linea ascendente (corrente > precedente)
            else
                prev_bar_char = ' '; // Questa cella deve restare vuota

            wprintf(L"%lc", prev_bar_char);

            // Stampa la linea superiore della barra dell'elemento corrente
            wprintf(L"%lc", data[i] == curr_height ? HORIZONTAL_BAR : L' ');
        }
        wprintf(L"\n");
    }

    wprintf(L"%lc", CORNER_BOTTOM_LEFT);
    for (int i = 0; i < term_width - 2; i++)
        wprintf(L"%lc", HORIZONTAL_BAR);
    wprintf(L">\n");
}
