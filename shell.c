//
// Created by Ondřej Váně on 28/12/2018.
//

#include <stdio.h>
#include "shell.h"

char *MESSAGE = "MESSAGE";

void print_message(char *string){
    printf("%s: %s\n", MESSAGE, string);
}
