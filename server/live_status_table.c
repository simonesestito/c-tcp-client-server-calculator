#include "live_status_table.h"
#include "calc_utils.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <wchar.h>
#include <arpa/inet.h>

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
 * Variabile globale per gestire finchè il server è in esecuzione
 */
int working = 1;

/**
 * Array degli elementi in visualizzazione nella tabella
 */
struct live_status_item **connection_items = NULL;

/**
 * Dimensione allocata del vettore.
 */
size_t connection_items_size = 1;

/**
 * Semaforo per la mutua esclusione nella sezione critica.
 * Regola l'accesso al vettore.
 */
pthread_mutex_t mutex;

/**
 * Thread per la visualizzazione della tabella
 */
pthread_t table_thread;

/**
 * Procedura in background per mostrare la tabella.
 * Si aggiorna ogni intervallo di millisecondi.
 */
void show_table(void) {
    while (working == 1) {
        pthread_mutex_lock(&mutex);
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
        for (size_t i = 0; connection_items != NULL && i < connection_items_size; i++) {
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
                    get_current_microseconds() / 1000000 - connection_items[i]->start_seconds,
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

        fflush(stdout);


        // TODO: Logger, ultime 5 righe

        funlockfile(stdout);
        pthread_mutex_unlock(&mutex);
        usleep(TABLE_MICROSECONDS_REFRESH);
    }

    // Mostra l'animazione durante la chiusura
    int visual_step = 0;
    while (working == 0) {
        char loading_char;
        switch (visual_step) {
            case 0:
                loading_char = '/';
                break;
            case 1:
                loading_char = '-';
                break;
            case 2:
                loading_char = '\\';
                break;
            case 3:
                loading_char = '|';
                break;
        }
        wprintf(L" %s %c    \r", "Chiusura in corso...", loading_char);
        fflush(stdout);
        visual_step = (visual_step + 1) % 4;
        usleep(TABLE_MICROSECONDS_REFRESH / 7);
    }
    wprintf(L"\n");
}

void init_status_table() {
    pthread_mutex_init(&mutex, NULL);
    pthread_create(&table_thread, NULL, (void *(*) (void *)) show_table, NULL);
}

void register_client(const struct sock_info *client, pthread_t thread_id) {
    pthread_mutex_lock(&mutex);

    // Crea la struttura
    struct live_status_item *item = malloc(sizeof(struct live_status_item));
    item->client = client;
    item->operations = 0;
    item->start_seconds = get_current_microseconds() / 1000000;
    item->thread_id = thread_id;

    if (connection_items == NULL)
        connection_items = calloc(connection_items_size, sizeof(struct live_status_item *));

    // Inserisci nella prima cella libera
    size_t i = 0;
    while (i < connection_items_size && connection_items[i] != NULL) { i++; }
    if (i == connection_items_size) {
        // Aumenta lo spazio allocato
        connection_items_size *= 2;
        connection_items = realloc(connection_items, connection_items_size);
    }
    connection_items[i] = item;

    pthread_mutex_unlock(&mutex);
}

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

    pthread_mutex_unlock(&mutex);
}

void add_client_operation(const struct sock_info *client) {
    pthread_mutex_lock(&mutex);

    // Trova il client nell'elenco
    for (size_t i = 0; i < connection_items_size; i++) {
        if (connection_items[i]->client == client) {
            // Aggiungi una nuova operazione
            connection_items[i]->operations++;
        }
    }

    pthread_mutex_unlock(&mutex);
}

void stop_status_table() {
    // Chiudi tutti i file descriptor
    // Join tutti i thread
    for (size_t i = 0; i < connection_items_size; i++) {
        if (connection_items == NULL)
            continue;

        struct live_status_item* item = connection_items[i];
        if (item == NULL)
            continue;

        if (pthread_join(item->thread_id, NULL) == -1)
            perror("Join thread in chiusura");

        // La free degli elementi del vettore viene fatto dal thread,
        // che quando finisci invoca la remove_client()
    }
    free(connection_items);

    // Interrompi anche l'animazione di chiusura
    working = -1;
    pthread_join(table_thread, NULL);

    pthread_mutex_destroy(&mutex);
}
