//
// Created by Ondřej Váně on 28/12/2018.
//

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>

#include "shell.h"
#include "file.h"
#include "global.h"
#include "shell_function.h"
#include "mft_tree.h"

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

char *MESSAGE = "MESSAGE";
int BUFF_SIZE = 100;
char token[2] = " ";

void print_message(char *string){
    printf("%s: %s\n", MESSAGE, string);
}

void shell(char *file_name){

    char *buffer = malloc(sizeof(char)*BUFF_SIZE);
    char *command;
    char *param1;
    char *param2;
    while(1){
        printf(ANSI_COLOR_GREEN   "%s:~ insert command>>> "   ANSI_COLOR_RESET , file_name);
        //printf("%s:~ insert command>>> ", file_name);
        //načtení řádky ze stdin
        fgets(buffer, BUFF_SIZE, stdin);
        if(strlen(buffer) == 1){
            continue;
        }

        if(buffer[0] == ' '){
            continue;
        }

        int i = (int) (strlen(buffer) - 1);
        if( buffer[ i ] == '\n') buffer[i] = '\0';

        //rozdělení vstupu na příkaz
        command = strtok(buffer, token);

        if(strcmp(command, "format") == 0){             //naformátuje aktuálně načtený soubor
            param1 = strtok(NULL, token);

            if(param1[strlen(param1)-1] == 'B' && param1[strlen(param1)-2] == 'M'){
               param1[strlen(param1)-2] = '\0';
            }
            else{
                printf("Wrong input!\n");
                continue;
            }
            format_file(file_name, atoi(param1));
            read_file(file_name);

        }
        else if(strcmp(command, "mkdir") == 0){       //vytvoří adresář a1

            param1 = strtok(NULL, token);               //a1
            function_mkdir(param1);

        }
        else if (strcmp(command, "pwd") == 0){        //vypíše aktuální cestu

            function_pwd();

        }
        else if(strcmp(command, "cd") == 0){          //změní aktuální cestu do adresáře

            param1 = strtok(NULL, token);
            function_cd(param1);

        }
        else if(strcmp(command, "rmdir") == 0){       //smaže prázdný adresář

            param1 = strtok(NULL, token);
            function_rmdir(param1);

        }
        else if(strcmp(command, "ls") == 0){          //vypíše obsah adresáře

            param1 = strtok(NULL, token);               //cesta
            function_ls(param1);

        }
        else if(strcmp(command, "incp") == 0){        //nahraje soubor s1 z pevného disku do umístění s2 v pseudoMTFS

            param1 = strtok(NULL, token);               //s1
            param2 = strtok(NULL, token);               //s2
            function_incp(param1, param2);

        }
        else if(strcmp(command, "outcp") == 0){       //nahraje soubor s1 z pseudoNTFS do umístění s2 na pevném disku

            param1 = strtok(NULL, token);               //s1
            param2 = strtok(NULL, token);               //s2
            function_outcp(param1, param2);

        }
        else if (strcmp(command, "rm") == 0){         //smaže soubor s1

            param1 = strtok(NULL, token);               //s1
            function_rm(param1);

        }
        else if(strcmp(command, "load") == 0){        //načte soubor z pevného disku, ve kterém budou jednotlivé příkazy
                                                        // a začne je sekvenčně vykonávat. Formát je 1 příkaz/ 1 rádek
            param1 = strtok(NULL, token);
            function_load(param1);

        }
        else if(strcmp(command, "cat") == 0){         //vypíše obsah souboru

            param1 = strtok(NULL, token);
            function_cat(param1);

        }
        else if(strcmp(command, "info") == 0){        //vypíše informace o souboru/adresáři s1/a1

            param1 = strtok(NULL, token);               //s1/a1
            function_info(param1);

        }
        else if(strcmp(command, "cp") == 0){          //zkopíruje soubor s1 do umístění s2

            param1 = strtok(NULL, token);               //s1
            param2 = strtok(NULL, token);               //s2
            function_cp(param1, param2);

        }
        else if(strcmp(command, "mv") == 0){        //přesune soubor s1 do umístění s2

            param1 = strtok(NULL, token);           //s1
            param2 = strtok(NULL, token);           //s2
            function_mv(param1, param2);

        }
        else if(strcmp(command, "slink") == 0){     //vytvoří symbolický link na soubor s1 s názvem s2

            param1 = strtok(NULL, token);           //s1
            param2 = strtok(NULL, token);           //s2
            function_slink(param1, param2);

        }
        else if(strcmp(command, "mojeinfo")==0){

            print_whole_tree(root_directory);
            bitmap_full();
            print_first_100_bitmap();

        }
        else if(strcmp(command, "clear") == 0){           //vyčištění obrazovky

            system("clear");

        }
        else if(strcmp(command, "test") == 0){            //moje testovací funkce

            param1 = strtok(NULL, token);
            function_test(param1);
        }
        else if(strcmp(command, "exit")==0){                //ukončení programu a uložení dat do soouboru
            printf("Writing global bitmap...\n");
            write_bitmap();
            printf("Writing global mft tree...\n");
            write_tree_to_file(root_directory);
            printf("Writing global boot record...\n");
            write_boot_record();
            printf("Closing myNTFS\n");
            fclose(global_file);
            free(global_bit_map);
            free(buffer);
            return;
        }
        else{
            printf("Command not found.\n");
        }
        memset(buffer,0,strlen(buffer));
    }

}
/**
 * Shell pro naformátování souboru pokud neexistuje.
 * Dokud se neprovede korektně naformátování nového
 * soboru, tak běží tento shell.
 * @param file_name název vytvářeného souboru
 * @return
 */
int shell_format(char *file_name){
    char *buffer = malloc(sizeof(char)*BUFF_SIZE);
    char token[2] = " ";
    char *command;
    char *param1;
    while (1){
        printf(ANSI_COLOR_GREEN   "%s:~ insert command \"format [size]MB\" >>> "   ANSI_COLOR_RESET , file_name);
        //načtení řádky ze stdin
        fgets(buffer, BUFF_SIZE, stdin);
        int i = (strlen(buffer)-1);
        if( buffer[ i ] == '\n') buffer[i] = '\0';

        //rozdělení vstupu na příkaz
        command = strtok(buffer, token);

        if(strcmp(command, "format") == 0){
            param1 = strtok(NULL, token);
            if(param1[strlen(param1)-1] == 'B' && param1[strlen(param1)-2] == 'M'){
                param1[strlen(param1)-2] = '\0';
            } else{
                printf("Wrong input!\n");
            }
            format_file(file_name, atoi(param1));
            free(buffer);
            return 1;
        } else if(strcmp(command, "help") == 0){
            print_message("Use command format [size]MB");
            print_message("Example -> format 600MB");
        } else if (strcmp(command, "exit") == 0){
            printf("Closing myNTFS\n");
            exit(EXIT_SUCCESS);
        } else{
            printf("Command not found.\n");
        }
        memset(buffer,0,strlen(buffer));
    }

}
