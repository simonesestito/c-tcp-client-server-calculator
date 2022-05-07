#ifndef HW2_IO_UTILS_H
#define HW2_IO_UTILS_H

#include "../common/calc_utils.h"
#include "../common/timestamp.h"
#include <stdio.h>

/**
 * Richiedi in input all'utente l'operazione da inviare al server
 *
 * @param left_operand Dove verrà memorizzato l'operando di sinistra
 * @param right_operand Dove verrà memorizzato l'operando di destra
 * @param operator Dove verrà memorizzato l'operatore
 * @return 0 se i dati sono stati ottenuti, -1 in caso non sia possibile neanche riprovare
*/
int get_user_input(operand_t *left_operand, operand_t *right_operand, char *operator);

/**
 * Invia i dati al server, se c'è ancora la connessione disponibile.
 *
 * @param socket_output FILE* dove comunicare col server
 * @param left_operand Operando di sinistra nell'operazione
 * @param right_operand Operando di destra nell'operazione
 * @param operator Operatore del calcolo
 * @return -1 in caso di errore, 0 altrimenti
 */
int send_operation_to_server(FILE *socket_output, const operand_t *left_operand, const operand_t *right_operand,
                             char operator);

/**
 * Ricevi il risultato dell'operazione dal server, ancora in raw, senza parsing.
 *
 * @param socket_input FILE* da cui ricevere i dati dal server
 * @param raw_server_line Stringa dove scrivere la risposta raw del server
 * @return -1 in caso di errore, 0 altrimenti
 */
int recv_operation_from_server(FILE *socket_input, char *raw_server_line);

/**
 * Esegui il parsing della linea restituita dal server, gestendo gli errori di ogni parte.
 *
 * @param raw_server_line Linea raw ricevuta dal server
 * @param start_time Tempo di inizio calcolo
 * @param end_time Tempo di fine calcolo
 * @param result Risultato dell'operazione
 * @param start_time_str Stringa del tempo di inizio calcolo
 * @param end_time_str Stringa del tempo di fine calcolo
 * @return -1 in caso di errore, 0 altrimenti
 */
int parse_server_result(const char *raw_server_line, struct timestamp *start_time, struct timestamp *end_time,
                        operand_t *result, char *start_time_str, char *end_time_str);

#endif //HW2_IO_UTILS_H
