//
// Created by Ondřej Váně on 02/01/2019.
//

#ifndef MYNTFS_SHELL_FUNCTION_H
#define MYNTFS_SHELL_FUNCTION_H

void function_mkdir(char *full_path);
void function_pwd();
void function_cd(char *full_path);
int get_free_cluster();
int is_name_duplicit(char *name, int uid);
int check_path(char *path);
void function_ls(char *full_path);
void function_rmdir(char *full_path);
void function_incp(char *pc_path, char *ntfs_path);
void function_outcp(char *ntfs_path, char *pc_path);
void function_rm(char *full_path);
void function_cat(char *full_path);
void function_info(char *full_path);
int get_first_cluster(int cluster_need);
void bitmap_full();
int check_relativ_path(char *path);
void print_first_100_bitmap();
char *get_file_name(char *full_path);

#endif //MYNTFS_SHELL_FUNCTION_H
