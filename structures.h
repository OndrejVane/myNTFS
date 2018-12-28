//
// Created by Ondřej Váně on 28/12/2018.
//

#ifndef MYNTFS_STRUCTURES_H
#define MYNTFS_STRUCTURES_H

#include <ntsid.h>
#include <stdbool.h>

const int32_t UID_ITEM_FREE = 0;
const int32_t MFT_FRAGMENTS_COUNT = 32;
const int32_t CLUSTER_SIZE = 10;
const int32_t CLUSTER_COUNT = 1024;
const int32_t MFT_FRAGMENT_FREE = -1;

struct boot_record {
    char signature[9];                                  //login autora FS
    char volume_descriptor[251];                        //popis vygenerovaného FS
    int32_t disk_size;                                  //celkova velikost VFS
    int32_t cluster_size;                               //velikost clusteru
    int32_t cluster_count;                              //pocet clusteru
    int32_t mft_start_address;                          //adresa pocatku mft
    int32_t bitmap_start_address;                       //adresa pocatku bitmapy
    int32_t data_start_address;                         //adresa pocatku datovych bloku
    int32_t mft_max_fragment_count;                     //maximalni pocet fragmentu v jednom zaznamu v mft (pozor, ne souboru)
                                                        //stejne jako   MFT_FRAGMENTS_COUNT
};

struct mft_fragment {
    int32_t fragment_start_address;                     //start adresa
    int32_t fragment_count;                             //pocet clusteru ve fragmentu
};

struct mft_item {
    int32_t uid;                                        //UID polozky, pokud UID = UID_ITEM_FREE, je polozka volna
    bool isDirectory;                                   //soubor, nebo adresar
    bool isSimbolicLink;                                // symbolikcý link nebo ne
    int8_t item_order;                                  //poradi v MFT pri vice souborech, jinak 1
    int8_t item_order_total;                            //celkovy pocet polozek v MFT
    char item_name[12];                                 //8+3 + /0 C/C++ ukoncovaci string znak
    int32_t item_size;                                  //velikost souboru v bytech
    struct mft_fragment fragments[MFT_FRAGMENTS_COUNT]; //fragmenty souboru
};

#endif //MYNTFS_STRUCTURES_H
