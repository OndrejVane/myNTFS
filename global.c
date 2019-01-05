//
// Created by Ondřej Váně on 02/01/2019.
//

#include "global.h"
#include "mft_tree.h"


FILE *global_file;
int8_t *global_bit_map;
struct boot_record *global_boot_record;
int pwd;
char pwd_path[200];
struct mft_node *root_directory;
struct mft_node *current_dirrectory;
const int32_t UID_ITEM_FREE = 0;
const int32_t MFT_FRAGMENTS_COUNT = 32;
const int32_t CLUSTER_SIZE = 10;
const int32_t MFT_FRAGMENT_FREE = -1;