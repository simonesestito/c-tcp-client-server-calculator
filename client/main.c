#include <stdlib.h>
#include "../common/socket_utils.h"
#include "../common/main_init.h"

int main(int argc, const char **argv) {
    // Inizializza
    if (main_init(argc, argv, "client.log", connect_to_server) != 0)
        return EXIT_FAILURE;
}