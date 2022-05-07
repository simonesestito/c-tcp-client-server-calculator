#ifndef SERVER_CALC_UTILS_H
#define SERVER_CALC_UTILS_H

#include <stddef.h>

/**
 * Tipo degli operandi
 */
typedef double operand_t;

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
double calculate_operation(operand_t left, char operator, operand_t right);

/**
 * Trova il numero minimo dall'array, non vuoto
 * @param data Array in cui trovare il minimo
 * @param data_len Dimensione dell'array
 * @return Il minimo numero trovato, o zero.
 */
unsigned int min(const unsigned int *data, size_t data_len);

/**
 * Trova il numero massimo dall'array, non vuoto
 * @param data Array in cui trovare il minimo
 * @param data_len Dimensione dell'array
 * @return Il massimo numero trovato
 */
unsigned int max(const unsigned int *data, size_t data_len);

#endif //SERVER_CALC_UTILS_H
