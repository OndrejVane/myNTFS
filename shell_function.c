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
    mft_item->item_size = 10;
    mft_item->parent_uid = path_temp;

    global_bit_map[free_cluster] = 1;

    // zapis fragmentu
    mft_item->fragments[0].fragment_start_address = global_boot_record->data_start_address + (free_cluster * global_boot_record->cluster_size);
    mft_item->fragments[0].fragment_count = 1; // pocet clusteru ve VFS od data start address
    mft_item->fragments[0].bitmap_start_possition = free_cluster;

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
            if(strcmp(mft_temp->mft_item->item_name, full_path) == 0 && mft_temp->mft_item->isDirectory == 1){
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
            } else{
                printf("PATH NOT FOUND\n");
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

void function_ls(char *full_path){
    int return_value;
    struct mft_node *temp = current_dirrectory->child;

    //vypsání obsahu aktuálního adresáře
    if (full_path == NULL){
        while (temp != NULL){
            if(temp->mft_item->isDirectory == 1){
                printf("+ %s\n", temp->mft_item->item_name);
            } else{
                printf("- %s\n", temp->mft_item->item_name);
            }
            temp = temp->next;
        }
        return;
    }

    //relativní cesta
    if(full_path[0] == '.'){
        //odstranění tečky
        full_path++;
        return_value = check_path(full_path);
        if(return_value != -1){
            temp = get_node_with_uid(root_directory, return_value);
            temp = temp->child;
            while (temp != NULL){
                if(temp->mft_item->isDirectory == 1){
                    printf("+ %s\n", temp->mft_item->item_name);
                } else{
                    printf("- %s\n", temp->mft_item->item_name);
                }
                temp = temp->next;
            }
        } else{
            printf("PATH NOT FOUND\n");
        }
        return;
    }

    //absolutní cesta
    if(full_path[0] == '/'){
        return_value =check_path(full_path);
        if (return_value != -1){
            temp = get_node_with_uid(root_directory, return_value);
            temp = temp->child;
            while (temp != NULL){
                if(temp->mft_item->isDirectory == 1){
                    printf("+ %s\n", temp->mft_item->item_name);
                } else{
                    printf("- %s\n", temp->mft_item->item_name);
                }
                temp = temp->next;
            }
        } else{
            printf("PATH NOT FOUND\n");
        }
        return;
    //podřazený adresář
    } else{
        while (temp != NULL){
            if (strcmp(temp->mft_item->item_name, full_path) == 0){
                temp = temp->child;
                while (temp != NULL){
                    if(temp->mft_item->isDirectory == 1){
                        printf("+ %s\n", temp->mft_item->item_name);
                    } else{
                        printf("- %s\n", temp->mft_item->item_name);
                    }
                    temp = temp->next;
                }
                return;
            }
            temp = temp->next;
        }
        printf("PATH NOT FOUND\n");
    }
}

void function_rmdir(char *full_path){
    struct mft_node *temp = current_dirrectory->child;
    int deleting_uid;
    int return_value;

    if (full_path == NULL){
        printf("PATH NOT FOUND\n");
        return;
    }

    //absolutní cesta
    if(full_path[0] == '/'){
        deleting_uid =check_path(full_path);
        if (deleting_uid != -1){
            return_value = delete_node_with_uid(deleting_uid);
            if (return_value == 1){
                printf("OK\n");
                return;
            }
            else if(return_value == -2){
                printf("NOT DIRECTORY\n");
                return;
            }
            else{
                printf("NOT EMPTY\n");
                return;
            }
        } else{
            printf("PATH NOT FOUND\n");
        }
    }

    //relativní cesta
    if(full_path[0] == '.'){
        //odstranění tečky
        full_path++;
        deleting_uid = check_path(full_path);
        if (deleting_uid != -1){
            return_value = delete_node_with_uid(deleting_uid);
            if (return_value == 1){
                printf("OK\n");
                return;
            } else if(return_value == -2){
                printf("NOT DIRECTORY\n");
                return;
            }else{
                printf("NOT EMPTY\n");
                return;
            }
        } else{
            printf("PATH NOT FOUND\n");
        }
        return;
    //podřazený adresář
    } else{
        while (temp != NULL){
            if (strcmp(temp->mft_item->item_name, full_path) == 0){
                deleting_uid = temp->mft_item->uid;
                return_value = delete_node_with_uid(deleting_uid);
                if (return_value == 1){
                    printf("OK\n");
                    return;
                } else if(return_value == -2){
                    printf("NOT DIRECTORY\n");
                    return;
                }else{
                    printf("NOT EMPTY\n");
                    return;
                }
            }
            temp = temp->next;
        }
        printf("PATH NOT FOUND\n");
    }
}

void function_incp(char *pc_path, char *ntfs_path){
    FILE *input_file;
    int32_t file_size;
    int32_t first_free_cluster; //index prvního clusteru, kam se vejde celý soubor
    int32_t number_of_clusters;
    char buffer[CLUSTER_SIZE];
    int uid_parent = -1;
    struct mft_node *temp;
    char *file_input_name;
    struct mft_item *new_item;      //nový mft item, který bude vkládán do stromu

    memset(buffer, 0, strlen(buffer));

    //pc_path je prázdná
    if(pc_path == NULL){
        printf("FILE NOT FOUND\n");
        return;
    }

    //cesta je prázdná
    if (ntfs_path == NULL){
        printf("PATH NOT FOUND\n");
        return;
    }

    //absolutní cesta v myNTFS
    if (ntfs_path[0] == '/'){
        uid_parent = check_path(ntfs_path);
    }

    if (ntfs_path[0] == '.'){
        //odstranění tečky
        ntfs_path++;
        uid_parent = check_path(ntfs_path);

    //podřazený adresář
    } else{
        temp = current_dirrectory->child;
        while (temp != NULL){
            if (strcmp(temp->mft_item->item_name, ntfs_path) == 0){
                uid_parent = temp->mft_item->uid;
                break;
            }
            temp = temp->next;
        }
    }

    //zkontrolování, zda existuje cílová cesta
    if(uid_parent == -1){
        printf("PATH NOT FOUND\n");
    }

    //načtení a kontrola vstupního souboru z počítače
    input_file = fopen(pc_path, "rb");
    if(input_file == NULL) {
        printf("FILE NOT FOUND\n");
        return;
    }

    //zjištění názvu vstupního souboru
    file_input_name = get_file_name(pc_path);

    //zjištění, zda již neexistuje soubor se stejným názvem
    if(is_name_duplicit(file_input_name, uid_parent) == -1){
        printf("EXISTS\n");
        return;
    }

    //zjištění velikosti souboru v bytech
    fseek(input_file, 0L, SEEK_END);
    file_size = (int32_t) ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    //zjištění potřebný počet clusterů
    if (file_size % CLUSTER_SIZE == 0){
        number_of_clusters = (int32_t) file_size/CLUSTER_SIZE;
    } else{
        number_of_clusters = (int32_t) (file_size/CLUSTER_SIZE) + 1;
    }

    //nalezení prvního volného clusteru, kam se vejde celý soubor
    first_free_cluster = get_first_cluster(number_of_clusters);
    if(first_free_cluster == -1){
        printf("NEVEJDE SE SOUVISLE DO CLUSTERŮ\n");
        return;
    }

    //vytvoření nového mft záznamu
    new_item = malloc(sizeof(struct mft_item));
    new_item->uid = global_boot_record->current_free_uid;
    global_boot_record->current_free_uid++;
    global_boot_record->number_of_fragments++;
    new_item->isDirectory = 0;
    new_item->isSimbolicLink = 0;
    strcpy(new_item->item_name, file_input_name);
    new_item->parent_uid = uid_parent;
    new_item->item_size = file_size;
    new_item->item_order_total = 1;
    new_item->item_order = 1;

    //zapsání všech plných clusterů do bitmapy
    for (int i = first_free_cluster; i < (first_free_cluster + number_of_clusters); i++) {
        global_bit_map[i] = 1;
    }
    //zapsání prvního fragmentu
    new_item->fragments[0].bitmap_start_possition = first_free_cluster;
    new_item->fragments[0].fragment_start_address = global_boot_record->data_start_address + (CLUSTER_SIZE*first_free_cluster);
    new_item->fragments[0].fragment_count = 1;

    for (int i = 1; i < MFT_FRAGMENTS_COUNT; i++){
        new_item->fragments[i].fragment_start_address = MFT_FRAGMENT_FREE;
        new_item->fragments[i].fragment_count = MFT_FRAGMENT_FREE;
    }

    //posun na záčátek data oblasti, kde budou zapisována data
    fseek(global_file, new_item->fragments[0].fragment_start_address, SEEK_SET);

    //načtení dat po CLUSTER SIZE, tedy po 10 bytech a zapsaání do myNTFS
    for (int j = 0; j < number_of_clusters; ++j) {
        fread(buffer,sizeof(char),10,input_file);
        fwrite(buffer, sizeof(char), 10, global_file);
        memset(buffer, 0, strlen(buffer));
    }

    //přidání záznamu do stromu
    add_next_under_uid(root_directory, uid_parent, new_item);

    printf("OK\n");
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
 * Funkce, která ze zadané cesty (absolutní i relativní) vrátí ukazatel
 * na název souboru
 * @param full_path zadaná cesta
 * @return název souboru
 */
char *get_file_name(char *full_path){
    char *return_value;
    if(full_path[0] == '.' || full_path[0] == '/'){
        return_value = strrchr(full_path, '/');
        return_value++;
    } else{
        return_value = full_path;
    }
    return return_value;
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
        while (temp_node != NULL){
            if(strcmp(temp, temp_node->mft_item->item_name) == 0 && temp_node->mft_item->isDirectory == 1){
                printf("Dir name: %s\n", temp_node->mft_item->item_name);
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

void bitmap_full(){
    int count = 0;
    for (int i = 0; i < global_boot_record->cluster_count ; ++i) {
        if (global_bit_map[i] == 1){
            count++;
        }
    }
    printf("BITMAP FULL: %d\n", count);
}

/**
 * Funkce, která nalezne v bitmapě tolik souvislých částí
 * které jsou předány v parametru. Pokud nenalezne dostatečný
 * počet clusterů za sebou vrací -1;
 * @param cluster_need
 * @return
 */
int get_first_cluster(int cluster_need){
    int max = global_boot_record->cluster_count;
    int free = 0;
    int first;


    for (int i = 0; i < max; ++i) {
        if(global_bit_map[i] == 0){

            if(free == 0){
                first = i;
            }
            free++;
            if(free == cluster_need){
                return first;
            }
        } else{
            free = 0;
            first = 0;
        }
    }
    return -1;
}

void print_first_100_bitmap(){
    for (int i = 0; i < 100; ++i) {
        printf("BITMAP[%d] = %d\n", i, global_bit_map[i]);
    }
}
