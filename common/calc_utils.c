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
