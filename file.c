//
// Created by Ondřej Váně on 27/12/2018.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "shell.h"
#include "structures.h"

void print_bit_map(int *bit_map);

void read_file(char *file_name){

    FILE *file;
    struct boot_record *boot_record;


    file = fopen(file_name, "rb");
    if(file == NULL){
        print_message("File not found.");
        create_file(file_name);
    } else{
        print_message("File found.");
    }

    file = fopen(file_name, "rb");
    boot_record = malloc(sizeof(struct boot_record));
    fread(boot_record, sizeof(struct boot_record), 1, file);

    printf("----------- BOOT RECORD -----------\n");
    printf("Signature: \t\t%s\n", boot_record->signature);
    printf("Description: \t\t%s\n", boot_record->volume_descriptor);
    printf("Disk size: \t\t%d\n", boot_record->disk_size);
    printf("Cluster count: \t\t%d\n", boot_record->cluster_count);
    printf("Cluster size: \t\t%d\n", boot_record->cluster_size);
    printf("MFT start address: \t%d\n", boot_record->mft_start_address);
    printf("Bit map start address: \t%d\n", boot_record->bitmap_start_address);
    printf("Data start address: \t%d\n", boot_record->data_start_address);
    printf("MFT fragments count: \t%d\n", boot_record->mft_max_fragment_count);
    printf("-----------------------------------\n");



}


void create_file(char *file_name){
    FILE *file;
    struct boot_record *boot_record;
    struct mft_item *mft_root_item;
    int bit_map[CLUSTER_COUNT];

    print_message("File is creating");

    // pomocne vypočty
    int size_of_mft = CLUSTER_COUNT * sizeof(struct mft_fragment);
    int size_of_bitmap = CLUSTER_COUNT * sizeof(int);


    //zápis do souboru
    file = fopen(file_name, "wb");
    if(file != NULL){
        boot_record = malloc(sizeof(struct boot_record));

        //vytvoření boot recordu
        strcpy(boot_record->signature, "vaneo");
        strcpy(boot_record->volume_descriptor, "myNTFS 2018-2019");
        boot_record->disk_size = CLUSTER_SIZE * CLUSTER_COUNT;
        boot_record->cluster_size = CLUSTER_SIZE;
        boot_record->cluster_count = CLUSTER_COUNT;
        boot_record->mft_start_address = sizeof(struct boot_record);
        boot_record->bitmap_start_address = sizeof(struct boot_record) + size_of_mft;
        boot_record->data_start_address = boot_record->bitmap_start_address + size_of_bitmap;
        boot_record->mft_max_fragment_count = MFT_FRAGMENTS_COUNT;

        //zapsání boot recordu do souboru
        fwrite(boot_record, sizeof(struct boot_record), 1, file);


        //posun na počátek umístění bitmapy
        fseek(file, boot_record->bitmap_start_address, SEEK_SET);


        //root directory
        bit_map[0] = 1;

        //ostatní clustery jsou volné
        for (int i = 1; i<CLUSTER_COUNT; i++){
            bit_map[i] = UID_ITEM_FREE;
        }

        //zápis bitmapy do souboru
        fwrite(bit_map, sizeof(int), CLUSTER_COUNT, file);


        //posun na začátek pozice mft oblasti
        fseek(file, sizeof(struct boot_record), SEEK_SET);

        //alokování paměti pro mft item
        mft_root_item = malloc(sizeof(struct mft_item));

        mft_root_item->uid = 0;
        mft_root_item->isDirectory = true;
        mft_root_item->isSimbolicLink = false;
        mft_root_item->item_order = 1;
        mft_root_item->item_order_total = 1;
        strcpy(mft_root_item->item_name, "ROOT");
        mft_root_item->item_size = 1;

        // zapis prvního fragmentu
        mft_root_item->fragments[0].fragment_start_address = boot_record->data_start_address;
        mft_root_item->fragments[0].fragment_count = 1; // pocet clusteru ve VFS od data start address


        for (int i = 1; i < MFT_FRAGMENTS_COUNT; i++){
            mft_root_item->fragments[i].fragment_start_address = MFT_FRAGMENT_FREE;
            mft_root_item->fragments[i].fragment_count = MFT_FRAGMENT_FREE;
        }

        fwrite(mft_root_item, sizeof(struct mft_item), 1, file);

        //uvolnění paměti
        free(boot_record);
        free(mft_root_item);


        fclose(file);



    }

    print_message("File is successfully created.");
}

void print_bit_map(int *bit_map){
    printf("\n*********** BIT MAP ***********\n");
    for (int i = 0; i < CLUSTER_COUNT; ++i) {
        printf("%d ", bit_map[i]);
        if(i % 15 == 0 && i != 0){
            printf("\n");
        }
    }
}
