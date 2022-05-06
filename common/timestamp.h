#ifndef HW2_TIMESTAMP_H
#define HW2_TIMESTAMP_H

#include <time.h>

/**
 * Dimensione della stringa per il solo tempo
 */
#define TIME_STRING_SIZE 20

/**
 * Dimensione della stringa per il timestamp fino ai microsecondi
 */
#define TIMESTAMP_STRING_SIZE (TIME_STRING_SIZE + 7)

/**
 * Formato della stringa per il tempo fino ai secondi
 */
#define TIME_STRING_FORMAT "%02d/%02d/%d %02d:%02d:%02d"

/**
 * Formato della stringa per il timestamp completo
 */
#define TIMESTAMP_STRING_FORMAT (TIME_STRING_FORMAT ".%06lu")

/**
 * Rappresenta tutti i dati di un timestamp
 */
struct timestamp {
    /**
     * Indicazioni del tempo, precisione al secondo
     */
    struct tm time;

    /**
     * Aggiungi informazioni sui microsecondi
     */
    long microseconds;
};

/**
 * Ottieni il timestamp corrente, fino ai microsecondi
 *
 * @param timestamp Timestamp corrente
 */
void get_timestamp(struct timestamp *timestamp);

/**
 * Trasforma il tempo in una stringa di dimensione TIME_STRING_SIZE
 *
 * @param time Tempo da scrivere
 * @param str Stringa dove scrivere
 */
void time_to_string(const struct tm *time, char *str);

/**
 * Trasforma il timestamp completo in una stringa di dimensione TIMESTAMP_STRING_SIZE
 *
 * @param timestamp Timestamp da scrivere
 * @param str Stringa dove scrivere
 */
void timestamp_to_string(const struct timestamp *timestamp, char *str);

/**
 * Ottieni il timestamp completo da una stringa
 *
 * @param timestamp Timestamp da scrivere
 * @param str Stringa contenente il timestamp formattato
 * @return -1 in caso di errore nella conversione, 0 altrimenti
 */
int string_to_timestamp(struct timestamp *timestamp, const char *str);

#endif //HW2_TIMESTAMP_H
