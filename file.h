//
// Created by Ondřej Váně on 27/12/2018.
//

#ifndef MYNTFS_FILE_H
#define MYNTFS_FILE_H

int read_file(char *file_name);
void write_bitmap();
void create_mft_tree(int number_of_fragments);
void write_mft_tree_to_file(struct mft_node *root);
void write_tree_to_file(struct mft_node *pNode);
void write_boot_record();
void print_global_boot_record();
void format_file(char *file_name, int size);

#endif //MYNTFS_FILE_H
