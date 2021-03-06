#include "live_status_table.h"
#include "../common/calc_utils.h"
#include "../common/logger.h"
#include "../common/main_init.h"
#include <stdlib.h>
#include <wchar.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

/**
 * Simboli Unicode per disegnare la tabella
 */
#define HORIZONTAL_BAR ((wchar_t) 0x2500)
#define VERTICAL_BAR ((wchar_t) 0x2502)
#define CORNER_BOTTOM_LEFT ((wchar_t) 0x2514)
#define CORNER_BOTTOM_RIGHT ((wchar_t) 0x2518)
#define CORNER_TOP_LEFT ((wchar_t) 0x250C)
#define CORNER_TOP_RIGHT ((wchar_t) 0x2510)
#define CROSS_CORNER ((wchar_t) 0x253C)
#define RIGHT_DIVIDER ((wchar_t) 0x251C)
#define LEFT_DIVIDER ((wchar_t) 0x2524)
#define TOP_DIVIDER ((wchar_t) 0x2534)
#define BOTTOM_DIVIDER ((wchar_t) 0x252C)

/**
 * Array degli elementi in visualizzazione nella tabella
 */
struct live_status_item **connection_items = NULL;

/**
 * Dimensione allocata del vettore.
 */
size_t connection_items_size = 1;

/**
 * Mutua esclusione nella sezione critica.
 * Regola l'accesso al vettore e alla pthread_cond_t.
 */
pthread_mutex_t mutex;

/**
 * Condizione per il refresh della tabella.
 * Di norma avviene comunque una volta al secondo.
 */
pthread_cond_t refresh_cond;

/**
 * Thread per la visualizzazione della tabella
 */
pthread_t table_thread;

/**
 * Procedura in background per mostrare la tabella.
 * Si aggiorna ogni intervallo di millisecondi.
 */
void show_table(void) {
    // Tempo massimo tra un refresh e l'altro
    struct timespec time_to_wait = {0, 0};

    while (socket_fd > 0) {
        // Attendi non piú di un secondo
        time_to_wait.tv_sec = time(NULL) + 1;
        pthread_mutex_lock(&mutex);
        pthread_cond_timedwait(&refresh_cond, &mutex, &time_to_wait);
        if (socket_fd <= 0) {
            // Esci in ogni caso
            pthread_mutex_unlock(&mutex);
            continue;
        }

        // Leggi orario attuale
        struct timestamp current_time;
        get_timestamp(&current_time);
        uint64_t current_seconds = timestamp_to_micros(&current_time) / 1000000;

        flockfile(stdout);

        // Pulisci schermo
        wprintf(L"\e[1;1H\e[2J");

        // Mostra intestazione tabella
        wprintf(L"%lc", CORNER_TOP_LEFT);
        for (int i = 0; i < 15; i++) wprintf(L"%lc", HORIZONTAL_BAR);
        wprintf(L"%lc", BOTTOM_DIVIDER);
        for (int i = 0; i < 5; i++) wprintf(L"%lc", HORIZONTAL_BAR);
        wprintf(L"%lc", BOTTOM_DIVIDER);
        for (int i = 0; i < 6; i++) wprintf(L"%lc", HORIZONTAL_BAR);
        wprintf(L"%lc", BOTTOM_DIVIDER);
        for (int i = 0; i < 5; i++) wprintf(L"%lc", HORIZONTAL_BAR);
        wprintf(L"%lc\n", CORNER_TOP_RIGHT);

        wprintf(L"%lc%-15s%lc%-5s%lc%-6s%lc%-5s%lc\n",
                VERTICAL_BAR,
                "Indirizzo IP",
                VERTICAL_BAR,
                "Porta",
                VERTICAL_BAR,
                "Op num",
                VERTICAL_BAR,
                "Tempo",
                VERTICAL_BAR);

        // Mostra le righe
        for (size_t i = 0; i < connection_items_size; i++) {
            if (connection_items[i] == NULL)
                continue;

            wprintf(L"%lc", RIGHT_DIVIDER);
            for (int j = 0; j < 15; j++) wprintf(L"%lc", HORIZONTAL_BAR);
            wprintf(L"%lc", CROSS_CORNER);
            for (int j = 0; j < 5; j++) wprintf(L"%lc", HORIZONTAL_BAR);
            wprintf(L"%lc", CROSS_CORNER);
            for (int j = 0; j < 6; j++) wprintf(L"%lc", HORIZONTAL_BAR);
            wprintf(L"%lc", CROSS_CORNER);
            for (int j = 0; j < 5; j++) wprintf(L"%lc", HORIZONTAL_BAR);
            wprintf(L"%lc\n", LEFT_DIVIDER);

            wprintf(L"%lc%-15s%lc%-5u%lc%-6u%lc%-5u%lc\n",
                    VERTICAL_BAR,
                    inet_ntoa(connection_items[i]->client->client_info.sin_addr),
                    VERTICAL_BAR,
                    htons(connection_items[i]->client->client_info.sin_port),
                    VERTICAL_BAR,
                    connection_items[i]->operations,
                    VERTICAL_BAR,
                    current_seconds - connection_items[i]->start_seconds,
                    VERTICAL_BAR);
        }

        // Chiudi la tabella
        wprintf(L"%lc", CORNER_BOTTOM_LEFT);
        for (int i = 0; i < 15; i++) wprintf(L"%lc", HORIZONTAL_BAR);
        wprintf(L"%lc", TOP_DIVIDER);
        for (int i = 0; i < 5; i++) wprintf(L"%lc", HORIZONTAL_BAR);
        wprintf(L"%lc", TOP_DIVIDER);
        for (int i = 0; i < 6; i++) wprintf(L"%lc", HORIZONTAL_BAR);
        wprintf(L"%lc", TOP_DIVIDER);
        for (int i = 0; i < 5; i++) wprintf(L"%lc", HORIZONTAL_BAR);
        wprintf(L"%lc\n", CORNER_BOTTOM_RIGHT);

        // Scrivi le ultime righe del log
        for (int i = 0; i < LOGS_ARRAY_SIZE; i++) {
            // Leggi dal vettore circolare
            int real_index = (logs_index + i) % LOGS_ARRAY_SIZE;
            if (logs_array[real_index] != NULL)
                wprintf(L"%s", logs_array[real_index]);
        }
        funlockfile(stdout);

        pthread_mutex_unlock(&mutex);
    }

    wprintf(L"\n");
}

/**
 * Inizializza la tabella
 */
void init_status_table() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&refresh_cond, NULL);
    pthread_create(&table_thread, NULL, (void *(*)(void *)) show_table, NULL);
    connection_items = calloc(connection_items_size, sizeof(struct live_status_item *));
}

/**
 * Registra un nuovo client in questa tabella
 *
 * @param client Informazioni sul client
 * @param thread_id ID POSIX del thread che lo sta gestendo
 */
void register_client(const struct sock_info *client, pthread_t thread_id) {
    pthread_mutex_lock(&mutex);

    // Leggi orario attuale
    struct timestamp current_time;
    get_timestamp(&current_time);
    uint64_t current_seconds = timestamp_to_micros(&current_time) / 1000000;

    // Crea la struttura
    struct live_status_item *item = malloc(sizeof(struct live_status_item));
    item->client = client;
    item->operations = 0;
    item->start_seconds = current_seconds;
    item->thread_id = thread_id;

    // Inserisci nella prima cella libera
    size_t i = 0;
    while (i < connection_items_size && connection_items[i] != NULL) { i++; }
    if (i == connection_items_size) {
        // Aumenta lo spazio allocato
        connection_items_size *= 2;
        connection_items = realloc(connection_items, connection_items_size * sizeof(struct live_status_item *));
    }
    connection_items[i] = item;

    pthread_cond_signal(&refresh_cond);
    pthread_mutex_unlock(&mutex);
}

/**
 * Rimuovi un client. Solitamente, quando il thread che lo gestiva sta terminando
 *
 * @param client Informazioni sul client gestito
 */
void remove_client(const struct sock_info *client) {
    pthread_mutex_lock(&mutex);

    // Trova il client nell'elenco
    for (size_t i = 0; i < connection_items_size; i++) {
        if (connection_items[i] != NULL && connection_items[i]->client == client) {
            // Rimuovi elemento
            void *item = connection_items[i];
            connection_items[i] = NULL;
            free(item);
        }
    }

    pthread_cond_signal(&refresh_cond);
    pthread_mutex_unlock(&mutex);
}

/**
 * Aggiungi una nuova operazione effettuata dal client
 *
 * @param client Client che effettua l'operazione
 */
void add_client_operation(const struct sock_info *client) {
    pthread_mutex_lock(&mutex);

    // Trova il client nell'elenco
    for (size_t i = 0; i < connection_items_size; i++) {
        if (connection_items[i] != NULL && connection_items[i]->client == client) {
            // Aggiungi una nuova operazione
            connection_items[i]->operations++;
        }
    }

    pthread_cond_signal(&refresh_cond);
    pthread_mutex_unlock(&mutex);
}

/**
 * Termina la visualizzazione della tabella,
 * e termina anche tutte le connessioni gestite,
 * attendendo i rispettivi thread.
 */
void stop_status_table() {
    // socket_fd è già stata impostata a zero nel main a questo punto.
    wprintf(L"\nChiusura in corso...\n");

    // Chiudi tutti i file descriptor
    // Join tutti i thread
    for (size_t i = 0; i < connection_items_size; i++) {
        struct live_status_item *item = connection_items[i];
        if (item == NULL)
            continue;

        // Sblocca il thread se era impegnato in una syscall bloccante (es: I/O con socket).
        // Queste due funzioni, come scritto nel man, restituiscono il numero di errore,
        // non restituendo -1 e poi sta in errno direttamente.
        if ((errno = pthread_kill(item->thread_id, SIGINT)) != 0)
            perror("pthread_kill in chiusura");

        // Attendi la sua fine
        if ((errno = pthread_join(item->thread_id, NULL)) != 0)
            perror("pthread_join in chiusura");

        // La free degli elementi del vettore viene fatto dal thread,
        // che quando finisci invoca la remove_client()
    }
    free(connection_items);

    // Attendi anche l'interruzione della tabella
    pthread_join(table_thread, NULL);

    pthread_cond_destroy(&refresh_cond);
    pthread_mutex_destroy(&mutex);
}
