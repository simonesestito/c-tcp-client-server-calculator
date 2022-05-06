#include "timestamp.h"
#include <stdio.h>

/**
 * Ottieni il timestamp corrente, fino ai microsecondi
 *
 * @param timestamp Timestamp corrente
 */
void get_timestamp(struct timestamp *timestamp) {
    time_t current_time = time(NULL);
    struct tm current_date = *localtime(&current_time);
    timestamp->time = current_date;

    struct timespec nanoseconds_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &nanoseconds_time);
    timestamp->microseconds = nanoseconds_time.tv_nsec / 1000;
}

/**
 * Trasforma il tempo in una stringa di dimensione TIME_STRING_SIZE
 *
 * @param time Tempo da scrivere
 * @param str Stringa dove scrivere
 */
void time_to_string(const struct tm *time, char *str) {
    snprintf(str, TIME_STRING_SIZE, TIME_STRING_FORMAT,
             time->tm_mday,
             time->tm_mon + 1,
             time->tm_year + 1900,
             time->tm_hour,
             time->tm_min,
             time->tm_sec);
}

/**
 * Trasforma il timestamp completo in una stringa di dimensione TIMESTAMP_STRING_SIZE
 *
 * @param timestamp Timestamp da scrivere
 * @param str Stringa dove scrivere
 */
void timestamp_to_string(const struct timestamp *timestamp, char *str) {
    snprintf(str, TIMESTAMP_STRING_SIZE, TIMESTAMP_STRING_FORMAT,
             timestamp->time.tm_mday,
             timestamp->time.tm_mon + 1,
             timestamp->time.tm_year + 1900,
             timestamp->time.tm_hour,
             timestamp->time.tm_min,
             timestamp->time.tm_sec,
             timestamp->microseconds);
}

/**
 * Ottieni il timestamp completo da una stringa
 *
 * @param timestamp Timestamp da scrivere
 * @param str Stringa contenente il timestamp formattato
 * @return -1 in caso di errore nella conversione, 0 altrimenti
 */
int string_to_timestamp(struct timestamp *timestamp, const char *str) {
    if (sscanf(str, TIMESTAMP_STRING_FORMAT,
               &(timestamp->time.tm_mday),
               &(timestamp->time.tm_mon),
               &(timestamp->time.tm_year),
               &(timestamp->time.tm_hour),
               &(timestamp->time.tm_min),
               &(timestamp->time.tm_sec),
               &(timestamp->microseconds)) < 7) {
        return -1;
    }

    // Aggiusta gli offset
    timestamp->time.tm_year -= 1900;
    timestamp->time.tm_mon -= 1;
    return 0;
}