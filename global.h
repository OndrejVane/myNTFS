//
// Created by Ondřej Váně on 02/01/2019.
//
#include <stdio.h>

#ifndef MYNTFS_GLOBAL_H
#define MYNTFS_GLOBAL_H

extern FILE *global_file;
extern int8_t *global_bit_map;
extern struct boot_record *global_boot_record;
extern int pwd;
extern char pwd_path[200];
extern struct mft_node *root_directory;
extern struct mft_node *current_dirrectory;
extern const int32_t UID_ITEM_FREE;
extern const int32_t MFT_FRAGMENTS_COUNT;
extern const int32_t CLUSTER_SIZE;
extern const int32_t MFT_FRAGMENT_FREE;

#endif //MYNTFS_GLOBAL_H
