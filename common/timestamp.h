#ifndef HW2_TIMESTAMP_H
#define HW2_TIMESTAMP_H

#include <time.h>
#include <stdint.h>

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
#define TIME_STRING_FORMAT "%02d/%02d/%04d %02d:%02d:%02d"

/**
 * Formato della stringa per il timestamp completo
 */
#define TIMESTAMP_STRING_FORMAT (TIME_STRING_FORMAT ".%06lu")

/**
 * Dimensione massima della stringa di differenza di timestamp.
 * [00h 00m 00s 0000000us]
 */
#define TIMEDIFF_STRING_SIZE 21

/**
 * Formato della stringa di differenza di timestamp
 */
#define TIMEDIFF_STRING_FORMAT "%02luh %02lum %02lus %06luus"

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
    uint64_t microseconds;
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

/**
 * Trasforma l'orario (non anche la data) in microsecondi
 *
 * @param timestamp Timestamp da cui prendere l'orario
 * @return Orario in microsecondi
 */
uint64_t timestamp_to_micros(const struct timestamp *timestamp);

/**
 * Formatta la differenza di due timestamp in una stringa
 * della grandezza TIMEDIFF_STRING_FORMAT.
 *
 * @param from Timestamp di partenza
 * @param to Timestamp di arrivo
 * @param str Stringa di destinazione della differenza
 */
void timediff_to_string(const struct timestamp *from, const struct timestamp *to, char *str);

#endif //HW2_TIMESTAMP_H
