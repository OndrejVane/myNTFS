//
// Created by Ondřej Váně on 02/01/2019.
//
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "shell_function.h"
#include "global.h"
#include "structures.h"
#include "mft_tree.h"

int pwd;

void function_mkdir(char *full_path){
    int length, path_temp, free_cluster;
    char *dir_name;
    char *path;
    struct mft_item *mft_item;

    if (full_path == NULL){
        printf("PATH NOT FOUND\n");
        return;
    }


    dir_name = strrchr(full_path, '/');
    //zadána cesta
    if(dir_name != NULL){
        //relativní
        if (full_path[0] == '.'){
            memmove(full_path, full_path+1, strlen(full_path));
            length = (int) (strlen(full_path) - strlen(dir_name));
            path = (char *) malloc(length);
            strncpy(path, full_path, length);
            path[length] = '\0';
            path_temp = check_path(path);
        } else{
            //absolutní cesta
            dir_name++;
            length = (int) (strlen(full_path) - strlen(dir_name));
            path = (char *) malloc(length);
            strncpy(path, full_path, length);
            path[length] = '\0';
            printf("ABSOLUT PATH: %s\n", path);
            path_temp = check_path(path);
        }
    } else{
        dir_name = full_path;
        path = (char *) malloc(2);
        path_temp = pwd;
    }

    //kontrola, zda je cesta validní
    if(path_temp == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    //kontrola, zda je název složky jedinečný v daném umístění
    if(is_name_duplicit(dir_name, path_temp) == -1){
        printf("EXISTS\n");
        return;
    }

    mft_item = malloc(sizeof(struct mft_item));

    free_cluster = get_free_cluster();

    if(free_cluster == -1){
        printf("NO MORE CLUSTERS\n");
        return;
    }

    mft_item->uid = global_boot_record->current_free_uid;
    global_boot_record->current_free_uid++;
    global_boot_record->number_of_fragments++;
    mft_item->isDirectory = true;
    mft_item->isSimbolicLink = false;
    mft_item->item_order = 1;
    mft_item->item_order_total = 1;
    strcpy(mft_item->item_name, dir_name);
    mft_item->item_size = 1;
    mft_item->parent_uid = path_temp;

    global_bit_map[free_cluster] = 1;

    // zapis fragmentu
    mft_item->fragments[0].fragment_start_address = global_boot_record->data_start_address + (free_cluster* global_boot_record->cluster_size);
    mft_item->fragments[0].fragment_count = 1; // pocet clusteru ve VFS od data start address

    for (int i = 1; i < MFT_FRAGMENTS_COUNT; i++){
        mft_item->fragments[i].fragment_start_address = MFT_FRAGMENT_FREE;
        mft_item->fragments[i].fragment_count = MFT_FRAGMENT_FREE;
    }

    add_next_under_uid(root_directory, path_temp, mft_item);

    free(path);

    printf("OK\n");

}

void function_cd(char *full_path){
    int return_value, length;
    char temp[200];
    strcpy(temp, full_path);
    struct mft_node *mft_temp;
    char *dir_name;



    if (full_path == NULL){
        printf("PATH NOT FOUND\n");
        return;
    }

    //jdu do nadřazené složky
    if(strcmp(full_path, "..") == 0){
        if(pwd != 0){
            pwd = current_dirrectory->mft_item->parent_uid;
            current_dirrectory = get_node_with_uid(root_directory, pwd);
            if (pwd == 0){
                memset(pwd_path,0,strlen(pwd_path));
                strcpy(pwd_path, "/\0");
                printf("OK\n");
            } else{
                dir_name = strrchr(pwd_path, '/');
                length = (int) (strlen(pwd_path) - strlen(dir_name));
                strcpy(temp, pwd_path);
                memset(pwd_path,0,strlen(pwd_path));
                for (int i = 0; i <length ; ++i) {
                    pwd_path[i] = temp[i];
                }
                //strncpy(pwd_path, pwd_path, length);
                pwd_path[length] = '\0';
                printf("OK\n");
            }
            return;
        }
        printf("PATH NOT FOUND\n");
        return;
    }

    //jdu do root dir
    if(strcmp(full_path, "/") == 0){
        pwd = 0;
        current_dirrectory = root_directory;
        memset(pwd_path,0,strlen(pwd_path));
        strcpy(pwd_path, "/\0");
        printf("OK\n");
        return;
    }

    //jdu podle relativní cesty
    if(full_path[0] == '.'){
        //odstranění tečky
        full_path++;
        memmove(temp, temp+1, strlen(temp));
        return_value = check_path(full_path);
        if(return_value != -1){
            pwd = return_value;
            strcat(pwd_path, temp);
            current_dirrectory = get_node_with_uid(root_directory, pwd);
            printf("OK\n");
        } else{
            printf("PATH NOT FOUND\n");
        }
        return;
    }

    //jdu podle absolutní cesty
    if(full_path[0] == '/'){
        return_value = check_path(full_path);
        if(return_value != -1){
            pwd = return_value;
            strcpy(pwd_path, temp);
            current_dirrectory = get_node_with_uid(root_directory, pwd);
            printf("OK\n");
        } else{
            printf("PATH NOT FOUND\n");
            return;
        }
    //jdu jen do následujícího adresáře
    } else{
        mft_temp = current_dirrectory->child;
        while (mft_temp != NULL){
            if(strcmp(mft_temp->mft_item->item_name, full_path) == 0){
                if (pwd == 0){
                    strcat(pwd_path, full_path);
                } else{
                    strcat(pwd_path, "/");
                    strcat(pwd_path, full_path);
                }
                pwd = mft_temp->mft_item->uid;
                current_dirrectory = mft_temp;
                printf("OK\n");
                return;
            }
            mft_temp = mft_temp->next;
        }
        printf("PATH NOT FOUND\n");
    }


}

void function_pwd(){

    printf("PATH: %s UID: %d\n", pwd_path, pwd);

}

int get_free_cluster(){
    for (int i = 1; i<global_boot_record->cluster_count; i++){
        if(global_bit_map[i] == 0){
            return i;
        }
    }
    return -1;
}

/**
 * Tato funkce zjistí zda název složky nebo souboru
 * již v dané podlsložce již existuje. Projde všechny
 * záznamy pod složkou s uid
 * @param name název souboru, který kontroluji
 * @param uid číslo složky, kterou procházím (parent UID)
 * @return -1 = je duplicitní 1 = OK
 */
int is_name_duplicit(char *name, int uid){
    struct mft_node *temp = get_node_with_uid(root_directory, uid);

    if (temp != NULL && temp->child != NULL){
        temp = temp->child;
        while (temp){
            if (strcmp(name, temp->mft_item->item_name) == 0){
                return -1;
            }
            temp = temp->next;
        }
        return 1;
    } else{
        return 1;
    }
}

/**
 * Funkce, která zkontroluje zda je zadaná cesta validní
 * pokud ano vrátí UID rodiče.
 * Kontroluje pouze absolutní cestu
 * @return UID rodiče nebo -1 cesta nenalezena
 */
int check_path(char *path){
    char token[2] = "/";
    char *temp;
    int current_uid = 0;
    struct mft_node *temp_node = current_dirrectory->child;

    //rozdělení na jednotlivé složky
     temp = strtok(path, token);


     while (temp != NULL){
        printf("TEST1\n");
        while (temp_node != NULL){
            printf("TEST2\n");
            if(strcmp(temp, temp_node->mft_item->item_name) == 0 && temp_node->mft_item->isDirectory == 1){
                printf("TEST3\n");
                current_uid = temp_node->mft_item->uid;
                temp_node = temp_node->child;
                temp = strtok(NULL, token);
                break;
            } else{
                printf("TEST4\n");
                temp_node = temp_node->next;
                if (temp_node == NULL){
                    return -1;
                }
            }
        }
     }
    return current_uid;
}

int check_relativ_path(char *path){
    char token[2] = "/";
    char *temp;
    int current_uid = 0;
    struct mft_node *temp_node = current_dirrectory->child;


    //rozdělení na jednotlivé složky
    temp = strtok(path, token);


    while (temp != NULL){
        while (temp_node != NULL){
            if(strcmp(temp, temp_node->mft_item->item_name) == 0 && temp_node->mft_item->isDirectory == 1){
                current_uid = temp_node->mft_item->uid;
                temp_node = temp_node->child;
                temp = strtok(NULL, token);
                break;
            } else{
                temp_node = temp_node->next;
                if (temp_node == NULL){
                    return -1;
                }
            }
        }
    }
    return current_uid;
}
