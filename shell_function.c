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
#include "file.h"

int pwd;

/**
 * Funkce, která vytvoří adresář.
 * @param full_path umístění adresáře
 */
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
            path = (char *) malloc((size_t) length);
            strncpy(path, full_path, length);
            path[length] = '\0';
            path_temp = check_relativ_path(path);
        } else{
            //absolutní cesta
            dir_name++;
            length = (int) (strlen(full_path) - strlen(dir_name));
            path = (char *) malloc((size_t) length);
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

    //kontorla, zda jsem našel volný cluster
    if(free_cluster == -1){
        printf("DISK IS FULL\n");
        return;
    }

    mft_item->uid = global_boot_record->current_free_uid;
    global_boot_record->current_free_uid++;
    global_boot_record->number_of_fragments++;
    mft_item->isDirectory = true;
    mft_item->isSymbolicLink = false;
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

    //ostatní fragmenty jsou prázdné
    for (int i = 1; i < MFT_FRAGMENTS_COUNT; i++){
        mft_item->fragments[i].fragment_start_address = MFT_FRAGMENT_FREE;
        mft_item->fragments[i].fragment_count = MFT_FRAGMENT_FREE;
    }

    //přidání do stromu
    add_next_under_uid(root_directory, path_temp, mft_item);

    free(path);

    printf("OK\n");
}

/**
 * Funkce, která změní aktuální cestu do adresáře.
 * @param full_path cesta k adresáři
 */
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
        return_value = check_relativ_path(full_path);
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
                mft_temp = mft_temp->next;
            }
        }
        printf("PATH NOT FOUND\n");
    }


}

/**
 * Fukce, která vypíše aktuální cestu k
 * adresáři, ve které se nachází.
 */
void function_pwd(){

    printf("%s \n", pwd_path);

}

/**
 * Funkce, která vypíše obsah adresáře
 * @param full_path cesta k adresáři pro výpis
 */
void function_ls(char *full_path){
    int return_value;
    struct mft_node *temp = current_dirrectory->child;

    //vypsání obsahu aktuálního adresáře
    if (full_path == NULL){
        while (temp != NULL){
            if(temp->mft_item->isDirectory == 1){
                printf("+ %s\n", temp->mft_item->item_name);
            } else{
                if(temp->mft_item->isSymbolicLink == 0){
                    printf("- %s\n", temp->mft_item->item_name);
                } else{
                    printf("-* %s\n", temp->mft_item->item_name);
                }

            }
            temp = temp->next;
        }
        return;
    }

    //relativní cesta
    if(full_path[0] == '.'){
        //odstranění tečky
        full_path++;
        return_value = check_relativ_path(full_path);
        if(return_value != -1){
            temp = get_node_with_uid(root_directory, return_value);
            temp = temp->child;
            while (temp != NULL){
                if(temp->mft_item->isDirectory == 1){
                    printf("+ %s\n", temp->mft_item->item_name);
                } else{
                    if(temp->mft_item->isSymbolicLink == 0){
                        printf("- %s\n", temp->mft_item->item_name);
                    } else{
                        printf("-* %s\n", temp->mft_item->item_name);
                    }
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
                    if(temp->mft_item->isSymbolicLink == 0){
                        printf("- %s\n", temp->mft_item->item_name);
                    } else{
                        printf("-* %s\n", temp->mft_item->item_name);
                    }
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
                        if(temp->mft_item->isSymbolicLink == 0){
                            printf("- %s\n", temp->mft_item->item_name);
                        } else{
                            printf("-* %s\n", temp->mft_item->item_name);
                        }
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

/**
 * Funkce, která smaže adresář (pouze pokud je prázdny).
 * @param full_path cesta k adresáři
 */
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
    //relativní cesta
    } else if(full_path[0] == '.'){
        //odstranění tečky
        full_path++;
        deleting_uid = check_relativ_path(full_path);
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

/**
 * Funcke, která načte soubor z pevného disku
 * a uloží do souborového systému
 * @param pc_path   cesta z pevného disku pc
 * @param ntfs_path     cesta v myNTFS
 */
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
        uid_parent = check_relativ_path(ntfs_path);

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
        return;
    }

    //načtení a kontrola vstupního souboru z počítače
    input_file = fopen(pc_path, "r+b");
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

    //nalezení prvního volného clusteru, kam se vejde vstupní soubor
    first_free_cluster = get_first_cluster(number_of_clusters);
    if(first_free_cluster == -1){
        printf("DISK IS FULL\n");
        return;
    }

    //vytvoření nového mft záznamu
    new_item = malloc(sizeof(struct mft_item));
    new_item->uid = global_boot_record->current_free_uid;
    global_boot_record->current_free_uid++;
    global_boot_record->number_of_fragments++;
    new_item->isDirectory = 0;
    new_item->isSymbolicLink = 0;
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

    //zavření vstupního souboru
    fclose(input_file);

    //přidání záznamu do stromu
    add_next_under_uid(root_directory, uid_parent, new_item);

    printf("OK\n");
}

/**
 * Funkce, která uloží soubor z myNTFS souboru na pevný
 * disk počítače
 * @param ntfs_path     cesta v myNTFS
 * @param pc_path       cesta na pevném disku pc
 */
void function_outcp(char *ntfs_path, char *pc_path){
    int parent_uid;
    int length;
    struct mft_node *temp;
    struct mft_node *output = NULL;
    char *file_name, *path;
    FILE *output_file;
    char *pc_path_and_file;

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

    //absolutní cesta
    if(ntfs_path[0] == '/'){
        file_name = strrchr(ntfs_path, '/');
        file_name++;
        length = (int) (strlen(ntfs_path) - strlen(file_name));
        path = (char *) malloc((size_t) length);
        strncpy(path, ntfs_path, length);
        path[length] = '\0';
        parent_uid = check_path(path);
    //relativní cesta
    }else if(ntfs_path[0] == '.'){
        file_name = strrchr(ntfs_path, '/');
        ntfs_path++;
        file_name++;
        length = (int) (strlen(ntfs_path) - strlen(file_name));
        path = (char *) malloc((size_t) length);
        strncpy(path, ntfs_path, length);
        path[length] = '\0';
        parent_uid = check_relativ_path(path);
    //jen soubor
    } else{
        parent_uid = pwd;
        file_name = ntfs_path;
    }

    //pokud není nalezená cesta, tak nemúže být ani soubor
    if(parent_uid == -1){
        printf("FILE NOT FOUND\n");
        return;
    }


    temp = get_node_with_uid(root_directory, parent_uid);
    temp = temp->child;

    //prohledání celé složky, zda zdrojový soubor existuje
    while(temp != NULL){
        if(strcmp(temp->mft_item->item_name, file_name) == 0 && temp->mft_item->isDirectory == 0){
            output = temp;
            break;
        }
        temp = temp->next;
    }


    //zkontrolování jestli jsem něco našel, pokud ne, tak je kopírování neúspěšné
    if(output == NULL){
        printf("FILE NOT FOUND\n");
        return;
    }

    //pokud se jedná o symblic link
    if (output->mft_item->isSymbolicLink != 0){
        output = get_node_with_uid(root_directory, output->mft_item->isSymbolicLink);
        if(output == NULL){
            printf("SYMBOLIC LINK IS BROKEN\n");
            return;
        }
    }

    //posun v disk na startovací pozici dat
    fseek(global_file, output->mft_item->fragments[0].fragment_start_address, SEEK_SET);

    //char buffer[output->mft_item->item_size];
    char *buffer = malloc(sizeof(char)*output->mft_item->item_size);
    memset(buffer, 0, output->mft_item->item_size);

    //načtení dat z myNTFS do bufferu
    fread(buffer, sizeof(char), (size_t)output->mft_item->item_size, global_file);

    pc_path_and_file = malloc(sizeof(char)*(strlen(pc_path) + strlen(file_name) +1));

    strcpy(pc_path_and_file, pc_path);
    strcat(pc_path_and_file, "/");
    strcat(pc_path_and_file, file_name);

    output_file = fopen(pc_path_and_file, "wb");
    if (output_file != NULL){
        for (int i = 0; i < output->mft_item->item_size; ++i) {
            fputc(buffer[i], output_file);
        }
        printf("OK\n");
        fclose(output_file);
    } else{
        printf("PATH NOT FOUND\n");
    }
    free(buffer);
    free(pc_path_and_file);
    buffer = NULL;
}

/**
 * Funkce, která vypíše obsah souboru na stdout.
 * @param full_path cesta k souboru
 */
void function_cat(char *full_path){
    int parent_uid;
    int length;
    struct mft_node *temp;
    struct mft_node *output = NULL;
    char *file_name, *path;

    //cesta je prázdná
    if (full_path == NULL){
        printf("PATH NOT FOUND\n");
        return;
    }

    //absolutní cesta
    if(full_path[0] == '/'){
        file_name = strrchr(full_path, '/');
        file_name++;
        length = (int) (strlen(full_path) - strlen(file_name));
        path = (char *) malloc(length);
        strncpy(path, full_path, length);
        path[length] = '\0';
        parent_uid = check_path(path);
    //relativní cesta
    }else if(full_path[0] == '.'){
        file_name = strrchr(full_path, '/');
        full_path++;
        file_name++;
        length = (int) (strlen(full_path) - strlen(file_name));
        path = (char *) malloc(length);
        strncpy(path, full_path, length);
        path[length] = '\0';
        parent_uid = check_relativ_path(path);
        //jen soubor
    } else{
        parent_uid = pwd;
        file_name = full_path;
    }

    //pokud není nalezená cesta, tak nemúže být ani soubor
    if(parent_uid == -1){
        printf("FILE NOT FOUND\n");
        return;
    }

    temp = get_node_with_uid(root_directory, parent_uid);
    temp = temp->child;

    //prohledání celé složky, zda zdrojový soubor existuje
    while(temp != NULL){
        if(strcmp(temp->mft_item->item_name, file_name) == 0 && temp->mft_item->isDirectory == 0){
            output = temp;
            break;
        }
        temp = temp->next;
    }

    //zkontrolování jestli jsem něco našel, pokud ne, tak je vypsání neúspěšné
    if(output == NULL){
        printf("FILE NOT FOUND\n");
        return;
    }

    //kontrola, zda se nejedná o symbolic link
    if(output->mft_item->isSymbolicLink != 0){
        output = get_node_with_uid(root_directory, output->mft_item->isSymbolicLink);
        if(output == NULL){
            printf("SYMBOLIC LINK IS BROKEN\n");
            return;
        }
    }

    //posun v disk na startovací pozici dat
    fseek(global_file, output->mft_item->fragments[0].fragment_start_address, SEEK_SET);

    char buffer[output->mft_item->item_size];
    memset(buffer, 0, strlen(buffer));

    fread(buffer, sizeof(char), (size_t)output->mft_item->item_size, global_file);

    for (int i = 0; i < output->mft_item->item_size; ++i) {
        printf("%c", buffer[i]);
    }
    printf("\n");
}

/**
 * Funkce, která smaže soubor z myNTFS.
 * @param full_path cesta k souboru
 */
void function_rm(char *full_path){
    char *file_name;
    char *path;
    int parent_uid;
    struct mft_node *temp;
    int length;

    if(full_path == NULL){
        printf("FILE NOT FOUND\n");
        return;
    }

    //absolutní cesta
    if(full_path[0] == '/'){
        file_name = strrchr(full_path, '/');
        file_name++;
        length = (int) (strlen(full_path) - strlen(file_name));
        path = (char *) malloc(length);
        strncpy(path, full_path, length);
        path[length] = '\0';
        parent_uid = check_path(path);
    //relativní cesta
    }else if(full_path[0] == '.'){
        file_name = strrchr(full_path, '/');
        full_path++;
        file_name++;
        length = (int) (strlen(full_path) - strlen(file_name));
        path = (char *) malloc(length);
        strncpy(path, full_path, length);
        path[length] = '\0';
        parent_uid = check_relativ_path(path);

    } else{
       parent_uid = pwd;
       file_name = full_path;
    }

    if(parent_uid == -1){
        printf("FILE NOT FOUND\n");
        return;
    }

    temp = get_node_with_uid(root_directory, parent_uid);
    temp = temp->child;

    while (temp != NULL){
        if(strcmp(temp->mft_item->item_name, file_name) == 0 && temp->mft_item->isDirectory == 0){

            if(delete_node_with_uid_file(temp->mft_item->uid) == 1){
                printf("OK\n");
                return;
            } else{
                printf("FILE NOT FOUND\n");
                return;
            }
        }
        temp = temp->next;
    }

    printf("FILE NOT FOUND\n");
}

/**
 * Funkce, která vypíše informace o adresáři/souboru.
 * @param full_path cesta k adresáři/souboru
 */
void function_info(char *full_path){
    char *file_name;
    int length;
    char *path;
    int parent_uid;
    struct mft_node *found_item = NULL;
    struct mft_node *temp;
    struct mft_node *slink = NULL;
    int number_of_clusters;
    int fragments_count = 0;
    //pokud je zadaná cesta prázdná, tak cesta neexistuje
    if (full_path == NULL){
        printf("FILE NOT FOUND\n");
        return;
    }

    //absolutní cesta
    if(full_path[0] == '/'){
        file_name = strrchr(full_path, '/');
        file_name++;
        length = (int) (strlen(full_path) - strlen(file_name));
        path = (char *) malloc(length);
        strncpy(path, full_path, length);
        path[length] = '\0';
        parent_uid = check_path(path);

    }else if(full_path[0] == '.'){

        file_name = strrchr(full_path, '/');
        full_path++;
        file_name++;
        length = (int) (strlen(full_path) - strlen(file_name));
        path = (char *) malloc(length);
        strncpy(path, full_path, length);
        path[length] = '\0';
        parent_uid = check_relativ_path(path);

    } else{
        parent_uid = pwd;
        file_name = full_path;
    }


    if(parent_uid == -1){
        printf("FILE NOT FOUND\n");
        return;
    }

    temp = get_node_with_uid(root_directory, parent_uid);
    temp = temp->child;

    //prohledání složky
    while (temp != NULL){
        if(strcmp(temp->mft_item->item_name, file_name) == 0){
            found_item = temp;
            break;
        }
        temp = temp->next;

    }

    //kontrola, zda byl nalezen prvek
    if (found_item == NULL){
        printf("FILE NOT FOUND\n");
        return;
    }

    if(found_item->mft_item->isSymbolicLink != 0){
        found_item = get_node_with_uid(root_directory, found_item->mft_item->isSymbolicLink);
        if(found_item == NULL){
            printf("SYMBOLIC LINK IS BROKEN\n");
            return;
        }
    }

    //zjištění počtu fragmentů
    for (int i = 0; i < MFT_FRAGMENTS_COUNT; ++i) {
        if (found_item->mft_item->fragments[i].fragment_start_address != MFT_FRAGMENT_FREE){
            fragments_count++;
        }
    }


    //zjištění počtu clusterů
    if (found_item->mft_item->item_size % CLUSTER_SIZE == 0){
        number_of_clusters = (int32_t) found_item->mft_item->item_size/CLUSTER_SIZE;
    } else{
        number_of_clusters = (int32_t) (found_item->mft_item->item_size/CLUSTER_SIZE) + 1;
    }

    char *a = found_item->mft_item->item_name;
    int32_t b = found_item->mft_item->uid;
    int32_t c = found_item->mft_item->item_size;
    int d = fragments_count;
    int e = number_of_clusters;




    if(found_item->mft_item->isDirectory == 1){
        printf("NAME: %s  TYPE: directory  UID: %d  SIZE: %d B  FRAGMENTS: %d  CLUSTERS COUNT: %d\n", a, b, c, d, e);
    } else{
        printf("NAME: %s  TYPE: file  UID: %d  SIZE: %d B  FRAGMENTS: %d  CLUSTERS COUNT: %d\n", a, b, c, d, e);
    }
}

void function_load(char *full_path){
    FILE *input_file;
    char * line = NULL;
    size_t len = 0;
    char token[2] = " ";
    char *command;
    char *param1;
    char *param2;


    input_file = fopen(full_path, "r");

    //kontrola, zda vstupní soubor existuje
    if (input_file == NULL){
        printf("FILE NOT FOUND\n");
        return;
    }

    //čtení ze souboru řádek po řádce
    while (getline(&line, &len, input_file) != -1) {

        //pokud se jedná o prázdnou řádku přeskakuju
        if (line[0] == '\n'){
            continue;
        }

        //odstraní z řetězce new line
        int i = (int) (strlen(line) - 1);
        if( line[ i ] == '\n') line[i] = '\0';

        //pokud se jedná o komentář, který je uvozený #, tak řádek přeskočím
        if (line[0] == '#'){
            continue;
        }

        printf("Command: %s ", line);
        command = strtok(line, token);


        if(strcmp(command, "format") == 0){             //naformátuje aktuálně načtený soubor
            param1 = strtok(NULL, token);

            if(param1[strlen(param1)-1] == 'B' && param1[strlen(param1)-2] == 'M'){
                param1[strlen(param1)-2] = '\0';
            }
            else{
                printf("Wrong input!\n");
                continue;
            }

            format_file(global_file_name, atoi(param1));
            read_file(global_file_name);

        } else if(strcmp(command, "mkdir") == 0){       //vytvoří adresář a1

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
        else{
            printf("Command not found.\n");
        }

    }

    printf("OK\n");

}

/**
 * Funkce která zkopíruje soubor v rámci myNTFS a
 * přesune ho do jiného umístění opět v rámci myNTFS
 * @param target_path cesta ke zdrojovému souboru
 * @param destination_path cesta pro uložení kopie souboru
 */
void function_cp(char *target_path, char *destination_path) {

    struct mft_item *new_item;
    int first_free_cluster;
    int number_of_clusters;
    char buffer[CLUSTER_SIZE];
    memset(buffer, 0, strlen(buffer));

    char *file_name;
    char *path;
    int length;
    int target_parent_uid, destination_parent_uid;
    struct mft_node *temp;
    struct mft_node *target_file = NULL;


    if (target_path == NULL){
        printf("FILE NOT FOUND\n");
        return;
    }

    if (destination_path == NULL){
        printf("PATH NOT FOUND");
    }

    //kontrola zdrojové cesty
    //absolutní cesta
    if(target_path[0] == '/'){
        file_name = strrchr(target_path, '/');
        file_name++;
        length = (int) (strlen(target_path) - strlen(file_name));
        path = (char *) malloc(length);
        strncpy(path, target_path, length);
        path[length] = '\0';
        target_parent_uid = check_path(path);
    //relativní cesta
    }else if(target_path[0] == '.'){

        file_name = strrchr(target_path, '/');
        target_path++;
        file_name++;
        length = (int) (strlen(target_path) - strlen(file_name));
        path = (char *) malloc(length);
        strncpy(path, target_path, length);
        path[length] = '\0';
        target_parent_uid = check_relativ_path(path);

    } else{
        target_parent_uid = pwd;
        file_name = target_path;
    }

    if(target_parent_uid == -1){
        printf("FILE NOT FOUND\n");
        return;
    }

    //nalezení zdrojového souboru v adresáři s uid = target_parent_uid
    temp = get_node_with_uid(root_directory, target_parent_uid);
    temp = temp->child;

    while (temp != NULL){
        if (strcmp(temp->mft_item->item_name, file_name) == 0 && temp->mft_item->isDirectory == 0){
            target_file = temp;
        }
        temp = temp->next;
    }

    //existuje target soubor v adresář??
    if(target_file == NULL){
        printf("FILE NOT FOUND\n");
        return;
    }

    //kontrola cílové adresy
    //absolutní cesta
    if(destination_path[0] == '/'){
        destination_parent_uid = check_path(destination_path);
        printf("DEST UID: %d\n", destination_parent_uid);
        //relativní cesta
    }else if(destination_path[0] == '.'){
        destination_path++;
        destination_parent_uid = check_relativ_path(destination_path);

    } else{
        destination_parent_uid = pwd;
    }

    //kontrola, zda existuje cílový adresář
    if(destination_parent_uid == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    //kontrola, zda již v cílovém adresáři neexistuje soubor se stejným nýzvem
    if(is_name_duplicit(file_name, destination_parent_uid) == -1){
        printf("EXISTS\n");
        return;
    }


    number_of_clusters = get_number_of_clusters(target_file->mft_item->item_size);

    //nalezení prvního volného clusteru, kam se vejde soubor
    first_free_cluster = get_first_cluster(number_of_clusters);
    if(first_free_cluster == -1){
        printf("DISK IS FULL\n");
        return;
    }


    //tady už existuje cíl i zdroj => můžu kopírovat
    //vytvořím nový mft záznam
    new_item = malloc(sizeof(struct mft_item));
    new_item->uid = global_boot_record->current_free_uid;
    global_boot_record->current_free_uid++;
    global_boot_record->number_of_fragments++;
    new_item->isDirectory = 0;
    if (target_file->mft_item->isSymbolicLink == 0){
        new_item->isSymbolicLink = 0;
    } else{
        //kopíruji symbolický link
        new_item->isSymbolicLink = target_file->mft_item->isSymbolicLink;
    }
    strcpy(new_item->item_name, target_file->mft_item->item_name);
    new_item->parent_uid = destination_parent_uid;
    new_item->item_size = target_file->mft_item->item_size;
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
    for (int j = 0; j < number_of_clusters; j++) {

        fseek(global_file, target_file->mft_item->fragments[0].fragment_start_address + (CLUSTER_SIZE * j), SEEK_SET);
        fread(buffer,sizeof(char),10,global_file);
        fseek(global_file, new_item->fragments[0].fragment_start_address + (CLUSTER_SIZE * j), SEEK_SET);
        fwrite(buffer, sizeof(char), 10, global_file);
        memset(buffer, 0, strlen(buffer));
    }

    //přidání záznamu do stromu
    add_next_under_uid(root_directory, destination_parent_uid, new_item);

    printf("OK\n");



}

/**
 * Funkce, která přesune soubor z umístění v myNTFS
 * na jíné místo v myNTFS.
 * @param target_path cesta ke zdrojovému souboru
 * @param destination_path cesta k cílovému adresáři
 */
void function_mv(char *target_path, char *destination_path){

    char *file_name;
    char *path;
    int length;
    int target_parent_uid, destination_parent_uid;
    struct mft_node *temp;
    struct mft_node *target_file = NULL;


    if (target_path == NULL){
        printf("FILE NOT FOUND\n");
        return;
    }

    if (destination_path == NULL){
        printf("PATH NOT FOUND");
    }

    //kontrola zdrojové cesty
    //absolutní cesta
    if(target_path[0] == '/'){
        file_name = strrchr(target_path, '/');
        file_name++;
        length = (int) (strlen(target_path) - strlen(file_name));
        path = (char *) malloc(length);
        strncpy(path, target_path, length);
        path[length] = '\0';
        target_parent_uid = check_path(path);
        //relativní cesta
    }else if(target_path[0] == '.'){

        file_name = strrchr(target_path, '/');
        target_path++;
        file_name++;
        length = (int) (strlen(target_path) - strlen(file_name));
        path = (char *) malloc(length);
        strncpy(path, target_path, length);
        path[length] = '\0';
        target_parent_uid = check_relativ_path(path);

    } else{
        target_parent_uid = pwd;
        file_name = target_path;
    }

    if(target_parent_uid == -1){
        printf("FILE NOT FOUND\n");
        return;
    }

    //nalezení zdrojového souboru v adresáři s uid = target_parent_uid
    temp = get_node_with_uid(root_directory, target_parent_uid);
    temp = temp->child;

    while (temp != NULL){
        if (strcmp(temp->mft_item->item_name, file_name) == 0 && temp->mft_item->isDirectory == 0){
            target_file = temp;
        }
        temp = temp->next;
    }

    //existuje target soubor v adresář??
    if(target_file == NULL){
        printf("FILE NOT FOUND\n");
        return;
    }

    //kontrola cílové adresy
    //absolutní cesta
    if(destination_path[0] == '/'){
        destination_parent_uid = check_path(destination_path);
        printf("DEST UID: %d\n", destination_parent_uid);
        //relativní cesta
    }else if(destination_path[0] == '.'){
        destination_path++;
        destination_parent_uid = check_relativ_path(destination_path);

    } else{
        destination_parent_uid = pwd;
    }

    //kontrola, zda existuje cílový adresář
    if(destination_parent_uid == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    //kontrola, zda již v cílovém adresáři neexistuje soubor se stejným nýzvem
    if(is_name_duplicit(file_name, destination_parent_uid) == -1){
        printf("EXISTS\n");
        return;
    }

    //funkce přesune uzel ze stromu a vloží na jíné místo
    move_node_with_uid(target_file->mft_item->uid, destination_parent_uid);
    printf("OK");
}


void function_slink(char *target_path, char *destination_path){

    char *target_file_name, *slink_file_name;
    char *path;
    char *path1;
    int length;
    int target_parent_uid, slink_parent_uid;
    struct mft_node *temp;
    struct mft_node *target_file = NULL;
    struct mft_item *slink;
    int free_cluster;


    if (target_path == NULL){
        printf("FILE NOT FOUND\n");
        return;
    }

    if (destination_path == NULL){
        printf("PATH NOT FOUND");
    }

    //kontrola zdrojové cesty
    //absolutní cesta
    if(target_path[0] == '/'){
        target_file_name = strrchr(target_path, '/');
        target_file_name++;
        length = (int) (strlen(target_path) - strlen(target_file_name));
        path = (char *) malloc(length);
        strncpy(path, target_path, length);
        path[length] = '\0';
        target_parent_uid = check_path(path);
        //relativní cesta
    }else if(target_path[0] == '.'){

        target_file_name = strrchr(target_path, '/');
        target_path++;
        target_file_name++;
        length = (int) (strlen(target_path) - strlen(target_file_name));
        path = (char *) malloc(length);
        strncpy(path, target_path, length);
        path[length] = '\0';
        target_parent_uid = check_relativ_path(path);

    } else{
        target_parent_uid = pwd;
        target_file_name = target_path;
    }

    if (path != NULL){
        free(path);
    }

    if(target_parent_uid == -1){
        printf("FILE NOT FOUND\n");
        return;
    }

    //nalezení zdrojového souboru v adresáři s uid = target_parent_uid
    temp = get_node_with_uid(root_directory, target_parent_uid);
    temp = temp->child;

    while (temp != NULL){
        if (strcmp(temp->mft_item->item_name, target_file_name) == 0 && temp->mft_item->isDirectory == 0){
            target_file = temp;
        }
        temp = temp->next;
    }

    //pokud není zdrojový soubor nalezen
    if(target_file == NULL){
        printf("FILE NOT FOUND\n");
        return;
    }


    //kontrola cílové cesty
    //absolutní cesta
    if(destination_path[0] == '/'){
        slink_file_name = strrchr(destination_path, '/');
        slink_file_name++;
        length = (int) (strlen(destination_path) - strlen(slink_file_name));
        path1 = (char *) malloc(length);
        strncpy(path1, destination_path, length);
        path1[length] = '\0';
        slink_parent_uid = check_path(path1);
        //relativní cesta
    }else if(destination_path[0] == '.'){

        slink_file_name = strrchr(destination_path, '/');
        destination_path++;
        slink_file_name++;
        length = (int) (strlen(destination_path) - strlen(slink_file_name));
        path1 = (char *) malloc(length);
        strncpy(path1, destination_path, length);
        path1[length] = '\0';
        slink_parent_uid = check_relativ_path(path1);

    } else{
        slink_parent_uid = pwd;
        slink_file_name = destination_path;
    }


    if(slink_parent_uid == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    printf("TEST\n");
    //kontorla, zda již soubor se stejným názvem neexistuje
    if (is_name_duplicit(slink_file_name, slink_parent_uid) == -1){
        printf("EXISTS\n");
        return;
    }

    free_cluster = get_free_cluster();

    //kontorla, zda jsem našel volný cluster
    if(free_cluster == -1){
        printf("DISK IS FULL\n");
        return;
    }

    global_bit_map[free_cluster] = 1;

    slink = malloc(sizeof(struct mft_item));
    slink->uid = global_boot_record->current_free_uid;
    global_boot_record->current_free_uid++;
    global_boot_record->number_of_fragments++;
    strcpy(slink->item_name, slink_file_name);
    slink->item_size = 10;
    slink->isDirectory = 0;
    slink->isSymbolicLink = target_file->mft_item->uid;
    slink->parent_uid = slink_parent_uid;
    slink->item_order = 1;
    slink->item_order_total = 1;


    // zapis fragmentu
    slink->fragments[0].fragment_start_address = global_boot_record->data_start_address + (free_cluster * global_boot_record->cluster_size);
    slink->fragments[0].fragment_count = 1; // pocet clusteru ve VFS od data start address
    slink->fragments[0].bitmap_start_possition = free_cluster;

    //ostatní fragmenty jsou prázdné
    for (int i = 1; i < MFT_FRAGMENTS_COUNT; i++){
        slink->fragments[i].fragment_start_address = MFT_FRAGMENT_FREE;
        slink->fragments[i].fragment_count = MFT_FRAGMENT_FREE;
    }

    add_next_under_uid(root_directory, slink_parent_uid, slink);

    printf("OK\n");
}
void function_test(char *full_path){
    printf("TEST: %s\n", full_path);
}

int32_t get_number_of_clusters(int file_size){
    //zjištění potřebný počet clusterů
    if (file_size % CLUSTER_SIZE == 0){
        return  (int32_t) file_size/CLUSTER_SIZE;
    } else{
        return  (int32_t) (file_size/CLUSTER_SIZE) + 1;
    }
}

/**
 * Funkce, která vráti první volný cluster,
 * na který narazí.
 * @return index volného clusteru
 */
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
    struct mft_node *temp_node = root_directory->child;

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
/**
 * Funkce porojde BITMAPU a spočíta, kolik
 * clusterů je zaplněno. Počet vypíše.
 */
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
