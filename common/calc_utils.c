#include "calc_utils.h"
#include <errno.h>

/**
 * Trova il numero minimo dall'array, non vuoto
 * @param data Array in cui trovare il minimo
 * @param data_len Dimensione dell'array
 * @return Il minimo numero trovato, o zero.
 */
unsigned int min(const unsigned int *data, size_t data_len) {
    if (data_len == 0)
        return 0;

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
    if (data_len == 0)
        return 0;

    unsigned long max_number = data[0];
    for (int i = 1; i < data_len; i++) {
        if (data[i] > max_number)
            max_number = data[i];
    }
    return max_number;
}

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
