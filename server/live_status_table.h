#ifndef SERVER_LIVE_STATUS_TABLE_H
#define SERVER_LIVE_STATUS_TABLE_H

#include "../common/socket_utils.h"

/**
 * Microsecondi dopo cui si deve aggiornare la tabella
 */
#define TABLE_MICROSECONDS_REFRESH (500 * 1000)

/**
 * Informazioni riguardo una connessione con un client,
 * e tutti i dettagli che verranno visualizzati
 * nella tabella di stato del server, come panoramica.
 */
struct live_status_item {
    /**
     * Informazioni sulla connessione col client
     */
    const struct sock_info *client;

    /**
     * ID del thread. L'ID pthread POSIX non verrà riciclato su altri thread nuovi.
     */
    pthread_t thread_id;

    /**
     * Tempo di inizio del thread
     */
    unsigned int start_seconds;

    /**
     * Numero di operazioni eseguite finora dal client
     */
    unsigned int operations;
};

/**
 * Inizializza la tabella
 */
void init_status_table();

/**
 * Registra un nuovo client in questa tabella
 *
 * @param client Informazioni sul client
 * @param thread_id ID POSIX del thread che lo sta gestendo
 */
void register_client(const struct sock_info *client, pthread_t thread_id);

/**
 * Rimuovi un client. Solitamente, quando il thread che lo gestiva sta terminando
 *
 * @param client Informazioni sul client gestito
 */
void remove_client(const struct sock_info *client);

/**
 * Aggiungi una nuova operazione effettuata dal client
 *
 * @param client Client che effettua l'operazione
 */
void add_client_operation(const struct sock_info *client);

/**
 * Termina la visualizzazione della tabella,
 * e termina anche tutte le connessioni gestite,
 * attendendo i rispettivi thread.
 */
void stop_status_table();

#endif //SERVER_LIVE_STATUS_TABLE_H
