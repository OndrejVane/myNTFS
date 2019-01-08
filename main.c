#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "global.h"
#include "file.h"
#include "shell.h"

/**
 * Vstupní funkce, která se spustí jako první. Zkontroluje
 * počet vstupních parametrů, načte(naformátuje) vstupní soubor
 * a spustí shell pro naslouchání příkazů.
 * @param argc počet vstupních parametrů
 * @param argv pole vstupních parametrů
 * @return
 */
int main(int argc, char *argv[]) {
    //zda zadaný vstupní soubor existuje nebo ne
    int return_value;

    //kontrola vstupních paramterů
    if(argc != 2){
        print_message("Wrong count of parameters!");
        print_message("EXIT");
        exit(EXIT_FAILURE);
    }

    //pokus o načtení souboru
    return_value = read_file(argv[1]);

    //start shellu pro naformátování nového souboru (pouze pokud neexistuje)
    if(return_value == -1){
        shell_format(argv[1]);
    }

    //načtení souboru, ted už existuje
    read_file(argv[1]);

    //start shellu pro naslouchání příkazů
    shell(argv[1]);

    return EXIT_SUCCESS;
}