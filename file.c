//
// Created by Ondřej Váně on 27/12/2018.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "structures.h"
#include "file.h"
#include "shell.h"
#include "global.h"
#include "mft_tree.h"

int read_file(char *file_name){

    FILE *file;
    struct boot_record *boot_record;
    int8_t *bit_map;
    struct mft_item *mft_item;

    file = fopen(file_name, "r+b");
    if(file == NULL){
        //print_message("File not found.");
        return -1;
    } else{
        //print_message("File found.");
    }
    fclose(file);
    file = fopen(file_name, "r+b");
    boot_record = malloc(sizeof(struct boot_record));
    fread(boot_record, sizeof(struct boot_record), 1, file);

    /*

    printf("----------- BOOT RECORD -----------\n");
    printf("Signature: \t\t%s\n", boot_record->signature);
    printf("Description: \t\t%s\n", boot_record->volume_descriptor);
    printf("Disk size: \t\t%dB\n", boot_record->disk_size);
    printf("Cluster count: \t\t%d\n", boot_record->cluster_count);
    printf("Cluster size: \t\t%dB\n", boot_record->cluster_size);
    printf("MFT start address: \t%d\n", boot_record->mft_start_address);
    printf("Bit map start address: \t%d\n", boot_record->bitmap_start_address);
    printf("Data start address: \t%d\n", boot_record->data_start_address);
    printf("MFT fragments count: \t%d\n", boot_record->mft_max_fragment_count);
    printf("Current free uid: \t%d\n", boot_record->current_free_uid);
    printf("Number of fragments: \t%d\n", boot_record->number_of_fragments);
    printf("-----------------------------------\n");
    */

    //načtení bitmapy do globální proměnné
    bit_map = malloc(sizeof(int8_t) * boot_record->cluster_count);
    fseek(file, boot_record->bitmap_start_address, SEEK_SET);
    fread(bit_map, sizeof(int8_t), (size_t) boot_record->cluster_count, file);
    global_bit_map = bit_map;

    //načtení kořenu do globální proměnné
    mft_item = malloc(sizeof(struct mft_item));
    //fseek(file, sizeof(struct boot_record), SEEK_SET);
    fseek(file, boot_record->mft_start_address, SEEK_SET);
    fread(mft_item, sizeof(struct mft_item), 1, file);


    global_boot_record = boot_record;


    //UID aktuální složky 0=root dir
    pwd = 0;
    strcpy(pwd_path, "/");

    global_file = file;

    create_mft_tree(global_boot_record->number_of_fragments);

    //pči načtení se nacházím v root dir
    current_dirrectory = root_directory;

    //print_whole_tree(root_directory);


    return 1;

}

void write_bitmap(){

    if(global_file != NULL){
        //posun na počátek umístění bitmapy
        fseek(global_file, global_boot_record->bitmap_start_address, SEEK_SET);
        //zapsání načtené bitmapy
        fwrite(global_bit_map, sizeof(int8_t), (size_t) global_boot_record->cluster_count, global_file);
        //uvolnění paměti po bit mapě
        free(global_bit_map);
        global_bit_map = NULL;
    } else{
        printf("ERROR\n");
    }
}

void write_boot_record(){

    //posun na začátek souboru
    fseek(global_file, 0, SEEK_SET);
    //zápis globálního boot recordu do souboru
    fwrite(global_boot_record, sizeof(struct boot_record), 1, global_file);
    //uvolnění paměti po boot recordu
    free(global_boot_record);
    global_boot_record = NULL;

    global_boot_record = malloc(sizeof(struct boot_record));
    fread(global_boot_record, sizeof(struct boot_record), 1, global_file);
}

void print_global_boot_record(){
    struct boot_record *boot_record = global_boot_record;
    printf("----------- BOOT RECORD -----------\n");
    printf("Signature: \t\t%s\n", boot_record->signature);
    printf("Description: \t\t%s\n", boot_record->volume_descriptor);
    printf("Disk size: \t\t%dB\n", boot_record->disk_size);
    printf("Cluster count: \t\t%d\n", boot_record->cluster_count);
    printf("Cluster size: \t\t%dB\n", boot_record->cluster_size);
    printf("MFT start address: \t%d\n", boot_record->mft_start_address);
    printf("Bit map start address: \t%d\n", boot_record->bitmap_start_address);
    printf("Data start address: \t%d\n", boot_record->data_start_address);
    printf("MFT fragments count: \t%d\n", boot_record->mft_max_fragment_count);
    printf("Current free uid: \t%d\n", boot_record->current_free_uid);
    printf("Number of fragments: \t%d\n", boot_record->number_of_fragments);
    printf("-----------------------------------\n");
}

void create_mft_tree(int number_of_fragments){

    struct mft_item *temp;
    fseek(global_file, global_boot_record->mft_start_address, SEEK_SET);

    for (int i = 0; i<number_of_fragments; i++){
        temp = malloc(sizeof(struct mft_item));
        fread(temp, sizeof(struct mft_item), 1, global_file);
        if(temp->parent_uid == -1){
            root_directory = new_node(temp);
        } else{
            add_next_under_uid(root_directory, temp->parent_uid,temp);
        }
        fseek(global_file, sizeof(struct mft_item), SEEK_CUR);
    }
}

void write_mft_tree_to_file(struct mft_node *root){
    struct mft_node *current1 = root;
    struct mft_node *current2;
    struct mft_node *temp;

    while (current1 != NULL){
        fwrite(current1->mft_item, sizeof(struct mft_item), 1, global_file);
        //printf("Return value mft: %d, cluster count: %d\n", return_value, global_boot_record->cluster_count);
        //printf("write: %d\n", current1->mft_item->uid);
        fseek(global_file, sizeof(struct mft_item), SEEK_CUR);
        current2 = current1;
        temp = current1->next;
        while (temp != NULL){
            write_mft_tree_to_file(current1->next);
            //current1 = current1->next;
            temp = NULL;
        }
        current1 = current2->child;
    }
}

void write_tree_to_file(struct mft_node *pNode) {
    //posun na začátek pozice mft oblasti
    fseek(global_file, sizeof(struct boot_record), SEEK_SET);
    //volání funkce pro zapsaání stromu do souboru
    write_mft_tree_to_file(pNode);
}


void format_file(char *file_name, int size){
    FILE *file;
    struct boot_record *boot_record;
    struct mft_item *mft_root_item;
    int8_t *bit_map;

    // pomocne vypočty
    int total_size = size * 1024 * 1024;                                                                //převod z MB na Bexit
    double temp_mft = total_size * 0.1 / sizeof(struct mft_item);                                       //velikost mft je 10% z celkové velikosti
    int size_of_mft = (int) (ceil(temp_mft) * sizeof(struct mft_item));                                 //zakroulení na celý mft záznam nahoru
    int cluster_count = (int) ceil((total_size - sizeof(struct boot_record) - size_of_mft) * 0.09);     //90% ze zbytku velikosti je určeno pro data, zaokrouhleno nahoru
    int size_of_bitmap = cluster_count * sizeof(int8_t);                                                //celkový počet clusterů

    //alokování bitmapy
    bit_map = malloc(cluster_count * sizeof(int8_t));

    //zápis do souboru
    file = fopen(file_name, "wb");
    if(file != NULL){
        //alokování boot recordu
        boot_record = malloc(sizeof(struct boot_record));

        //vytvoření boot recordu
        strcpy(boot_record->signature, "vaneo");
        strcpy(boot_record->volume_descriptor, "myNTFS 2018-2019");
        boot_record->mft_start_address = sizeof(struct boot_record);
        boot_record->bitmap_start_address = sizeof(struct boot_record) + size_of_mft;
        boot_record->disk_size = CLUSTER_SIZE * cluster_count;
        boot_record->cluster_size = CLUSTER_SIZE;
        boot_record->cluster_count = cluster_count;
        boot_record->data_start_address = boot_record->bitmap_start_address + size_of_bitmap;
        boot_record->mft_max_fragment_count = MFT_FRAGMENTS_COUNT;
        boot_record->current_free_uid = 1;
        boot_record->number_of_fragments = 1;

        //zapsání boot recordu do souboru
        fwrite(boot_record, sizeof(struct boot_record), 1, file);

        //posun na počátek umístění bitmapy
        fseek(file, boot_record->bitmap_start_address, SEEK_SET);


        //root directory
        bit_map[0] = 1;

        //ostatní clustery jsou volné
        for (int i = 1; i<cluster_count; i++){
            bit_map[i] = UID_ITEM_FREE;
        }

        //zápis bitmapy do souboru
        fwrite(bit_map, sizeof(int8_t), (size_t) cluster_count, file);


        //posun na začátek pozice mft oblasti
        fseek(file, sizeof(struct boot_record), SEEK_SET);


        //alokování paměti pro mft item
        mft_root_item = malloc(sizeof(struct mft_item));

        mft_root_item->uid = 0;
        mft_root_item->isDirectory = true;
        mft_root_item->isSymbolicLink = false;
        mft_root_item->item_order = 1;
        mft_root_item->item_order_total = 1;
        strcpy(mft_root_item->item_name, "ROOT");
        mft_root_item->item_size = 10;
        mft_root_item->parent_uid = -1;

        // zapis prvního fragmentu
        mft_root_item->fragments[0].fragment_start_address = boot_record->data_start_address;
        mft_root_item->fragments[0].fragment_count = 1; // pocet clusteru ve VFS od data start address
        mft_root_item->fragments[0].bitmap_start_possition = 0;


        for (int i = 1; i < MFT_FRAGMENTS_COUNT; i++){
            mft_root_item->fragments[i].fragment_start_address = MFT_FRAGMENT_FREE;
            mft_root_item->fragments[i].fragment_count = MFT_FRAGMENT_FREE;
        }

        //zapsání root mft itemu do souboru
        fwrite(mft_root_item, sizeof(struct mft_item), 1, file);

        //posun na konec celého souboru a zapsání EOF
        fseek(file, boot_record->data_start_address + boot_record->disk_size, SEEK_SET);
        putc(EOF, file);

        /*
        //zapsání odkazu na nadřazený adresář
        fseek(file, boot_record->data_start_address, SEEK_SET);
        char link[2];
        link[0] = '0';
        link[1] = '\0';
        fwrite(link, 1, 2, file);
         */


        //uvolnění paměti
        free(boot_record);
        free(mft_root_item);
        free(bit_map);


        fclose(file);
        printf("OK\n");
    } else{
        printf("CANNOT CREATE FILE\n");
    }
}
