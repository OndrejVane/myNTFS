//
// Created by Ondřej Váně on 28/12/2018.
//
#include <ntsid.h>
#include <stdio.h>
#include <stdbool.h>
#include "global.h"

#ifndef MYNTFS_STRUCTURES_H
#define MYNTFS_STRUCTURES_H

struct boot_record {
    char signature[9];                                  //login autora FS
    char volume_descriptor[251];                        //popis vygenerovaného FS
    int32_t disk_size;                                  //celkova velikost VFS
    int32_t cluster_size;                               //velikost clusteru
    int32_t cluster_count;                              //pocet clusteru
    int32_t mft_start_address;                          //adresa pocatku mft
    int32_t bitmap_start_address;                       //adresa pocatku bitmapy
    int32_t data_start_address;                         //adresa pocatku datovych bloku
    int32_t mft_max_fragment_count;                     //maximalni pocet fragmentu v jednom zaznamu v mft (pozor, ne souboru)stejne jako   MFT_FRAGMENTS_COUNT
    int32_t current_free_uid;
    int32_t number_of_fragments;
};

struct mft_fragment {
    int32_t fragment_start_address;                     //start adresa
    int32_t fragment_count;                             //pocet clusteru ve fragmentu
    int32_t bitmap_start_possition;                     //první pozice v bitmapě
};

struct mft_item {
    int32_t uid;                                        //UID polozky, pokud UID = UID_ITEM_FREE, je polozka volna
    bool isDirectory;                                   //soubor, nebo adresar
    int32_t isSymbolicLink;                             //pokud je slink tak zde bude id na co ukazuji
    int8_t item_order;                                  //poradi v MFT pri vice souborech, jinak 1
    int8_t item_order_total;                            //celkovy pocet polozek v MFT
    char item_name[12];                                 //8+3 + /0 C/C++ ukoncovaci string znak
    int32_t item_size;                                  //velikost souboru v bytech
    int32_t parent_uid;                                 //odkaz na nadřazený adresář
    struct mft_fragment fragments[32];                  //fragmenty souboru
};

/**
 * Pomocná struktura pro návratovou hodnotu
 * path = cesta k souboru/složce
 * name = název souboru/složky
 */
struct inf_box{
    int32_t uid;
    char *name;
    struct mft_node *mft_node;
};



#endif //MYNTFS_STRUCTURES_H
