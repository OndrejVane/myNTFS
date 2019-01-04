//
// Created by Ondřej Váně on 03/01/2019.
//

#include <stdlib.h>
#include "mft_tree.h"
#include "structures.h"

struct mft_node *new_node(struct mft_item *mft_item)
{
    struct mft_node *new_node = malloc(sizeof(struct mft_node));

    if ( new_node ) {
        new_node->next = NULL;
        new_node->child = NULL;
        new_node->mft_item = mft_item;
    }
    return new_node;
}

void add_child(struct mft_node * root, struct mft_item *mft_item)
{
    struct mft_node * current = root;
    while (current->child != NULL) {
        current = current->child;
    }

    current->child = new_node(mft_item);
}

struct mft_node *get_node_with_uid(struct mft_node *root, int uid)
{
    struct mft_node *current1 = root;
    struct mft_node *current2;
    struct mft_node *temp;
    while (current1 != NULL){
        if(current1->mft_item->uid == uid){
            return current1;
        }
        current2 = current1;
        while (current1->next != NULL){
            temp = get_node_with_uid(current1->next, uid);
            if (temp != NULL){
                return temp;
            }
            current1 = current1->next;
        }
        current1 = current2->child;
    }
    return NULL;
}

void add_next_under_uid(struct mft_node *root, int uid, struct mft_item *mft_item)
{
    struct mft_node *found_node = get_node_with_uid(root, uid);
    struct mft_node *adding_node = new_node(mft_item);
    struct mft_node *current;

    current = found_node->child;
    if(current == NULL){
        found_node->child = adding_node;
    } else{
        while (current->next != NULL){
            current = current->next;
        }
        current->next = adding_node;
    }
}

void print_whole_tree(struct mft_node *root){
    struct mft_node *current1 = root;
    struct mft_node *current2;
    struct mft_node *temp;
    while (current1 != NULL){
        printf("Name: %s UID: %d PUID: %d\n",current1->mft_item->item_name, current1->mft_item->uid, current1->mft_item->parent_uid);
        current2 = current1;
        temp = current1->next;
        while (temp != NULL){
            print_whole_tree(current1->next);
            //current1 = current1->next;
            temp = NULL;
        }
        current1 = current2->child;
    }
}
/**
 * Funkce, která smaže ze stromu záznam se zadaným UID.
 * Smaže tento záznam jen tehdy, když neobsahuje žádné potomky.
 * @param uid_delete
 * @return -1 = obsahuje potomky, 1= úspěšně smazán
 */
int delete_node_with_uid(int uid_delete){
    struct mft_node *deleting_note = get_node_with_uid(root_directory, uid_delete);
    struct mft_node *deteting_parent = get_node_with_uid(root_directory, deleting_note->mft_item->parent_uid);
    struct mft_node *temp = deteting_parent->child;
    struct mft_node *previous_note = NULL;

    //neobsahuje potomky => můžu smazat
    if (deleting_note->child == NULL){
        while (temp != NULL){
            if(temp->mft_item->uid == uid_delete){
                //mažu přímého potomka rodiče
                if(previous_note == NULL){
                    deteting_parent->child = temp->next;
                    free(temp->mft_item);
                    free(temp);
                    global_boot_record->number_of_fragments--;
                    return 1;
                } else{
                    previous_note->next = temp->next;
                    free(temp->mft_item);
                    free(temp);
                    global_boot_record->number_of_fragments--;
                    return 1;
                }
            }
            previous_note = temp;
            temp = temp->next;
        }
        global_boot_record->number_of_fragments--;
        return 1;
    } else{
        //obsahuje potomky, nelze smazat
        return -1;
    }
}