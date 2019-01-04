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
        printf("%s:~ insert command>>> ", file_name);
        //načtení řádky ze stdin
        fgets(buffer, BUFF_SIZE, stdin);
        if(strlen(buffer) == 1){
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
        } else if(strcmp(command, "mkdir") == 0){       //vytvoří adresář

            param1 = strtok(NULL, token);
            function_mkdir(param1);

        } else if (strcmp(command, "pwd") == 0){        //vypíše aktuální cestu

            function_pwd();

        } else if(strcmp(command, "cd") == 0){          //změní aktuální cestu do adresáře

            param1 = strtok(NULL, token);
            function_cd(param1);

        } else if(strcmp(command, "rmdir") == 0){       //smaže prázdný adresář

            param1 = strtok(NULL, token);
            function_rmdir(param1);

        } else if(strcmp(command, "ls") == 0){          //vypíše obsah adresáře

            param1 = strtok(NULL, token);
            function_ls(param1);

        }
        else if(strcmp(command, "info")==0){
            print_whole_tree(root_directory);
            bitmap_full();
        }
        else if(strcmp(command, "help")==0){
            printf("exit:\tending the program\n");
            printf("format <size MB>:\t format new file with size of MB\n");
        }
        else if(strcmp(command, "exit")==0){
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
        printf("%s:~ insert command \"format [size]MB\" >>> ", file_name);
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
