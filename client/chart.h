#ifndef HW2_CHART_H
#define HW2_CHART_H

#include <stddef.h>

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
 * Mostra sul terminale un grafico con i dati forniti in input
 *
 * @param data Dati da mostrare
 * @param data_len Dimensione dei dati
 */
void plot_chart(const unsigned int *original_data, size_t data_len);

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
                int max_data_len, size_t data_len);

/**
 * Disegna una singola cella, grande 1x2, nel grafico
 *
 * @param data Dati di origine del grafico
 * @param i Indice della riga della cella
 * @param curr_height Altezza corrente nel grafico
 */
void draw_single_cell(const unsigned int *data, int i, int curr_height);

#endif //HW2_CHART_H
