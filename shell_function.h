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
int check_relativ_path(char *path);

#endif //MYNTFS_SHELL_FUNCTION_H
