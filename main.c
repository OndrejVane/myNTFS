#include <stdio.h>
#include <stdlib.h>
#include "file.h"


int main(int argc, char *argv[]) {

    //kontrola vstuních paramterů
    if(argc != 2){
        printf("Wrong count of parameters!\n");
        printf("EXIT\n");
        exit(-1);
    }

    //načtení nebo vytvoření souboru
    read_file(argv[1]);





    //TODO spuštění konzole pro načítání příkazů
    exit(0);
}
