#include <wchar.h>
#include <locale.h>

int main() {
    // Dato che verranno usati wchar_t, il terminale deve essere
    // in modalità "wide charset" e serve impostare il locale.
    // Tutte le printf dovranno essere wprintf, altrimenti
    // il comportamento non è definito nelle specifiche del C
    setlocale(LC_CTYPE, "");


    wprintf(L"Sono il client!\n");
}