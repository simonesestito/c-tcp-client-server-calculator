#ifndef SERVER_CALC_UTILS_H
#define SERVER_CALC_UTILS_H

#include <stdint.h>

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
 * Ottieni il tempo in microsecondi attuali
 *
 * @return Tempo corrente in microsecondi
 */
uint64_t get_current_microseconds();

#endif //SERVER_CALC_UTILS_H
