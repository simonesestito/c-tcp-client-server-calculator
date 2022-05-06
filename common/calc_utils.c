#include "calc_utils.h"
#include <errno.h>
#include <time.h>

/**
 * Elabora il calcolo richiesto dal client.
 *
 * Imposta errno in caso di errore.
 *
 * @param left Operando di sinistra
 * @param operator Operatore
 * @param right Operando di destra
 * @return Il risultato dell'operazione, oppure 0 in caso di errore impostando errno.
 */
double calculate_operation(operand_t left, char operator, operand_t right) {
    errno = 0;
    switch (operator) {
        case '+':
            return left + right;
        case '-':
            return left - right;
        case '*':
            return left * right;
        case '/':
            return left / right;
        default:
            errno = EINVAL;
            return 0;
    }
}

/**
 * Ottieni il tempo in microsecondi attuali
 *
 * @return Tempo corrente in microsecondi
 */
uint64_t get_current_microseconds() {
    struct timespec current_time;
    // TODO: Deve essere user-friendly gg/mm/yyyy hh:mm:ss.micros
    clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
    return current_time.tv_sec * 1000 * 1000 + current_time.tv_nsec / 1000;
}