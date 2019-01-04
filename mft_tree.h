//
// Created by Ondřej Váně on 03/01/2019.
//


#ifndef MYNTFS_MFT_TREE_H
#define MYNTFS_MFT_TREE_H

struct mft_node {
    struct mft_item *mft_item;
    struct mft_node *next;
    struct mft_node *child;
};

struct mft_node *new_node(struct mft_item *mft_item);
void add_child(struct mft_node * root, struct mft_item *mft_item);
struct mft_node *get_node_with_uid(struct mft_node *root, int uid);
void add_next_under_uid(struct mft_node *root, int uid, struct mft_item *mft_item);
void print_whole_tree(struct mft_node *root);

#endif //MYNTFS_MFT_TREE_H
