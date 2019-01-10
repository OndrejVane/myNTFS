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
 * @return -1 = obsahuje potomky, 1= úspěšně smazán, -2 = není adresář
 */
int delete_node_with_uid(int uid_delete){
    struct mft_node *deleting_note = get_node_with_uid(root_directory, uid_delete);
    struct mft_node *deteting_parent = get_node_with_uid(root_directory, deleting_note->mft_item->parent_uid);
    struct mft_node *temp = deteting_parent->child;
    struct mft_node *previous_note = NULL;
    int bitmap_possition = deleting_note->mft_item->fragments[0].bitmap_start_possition;

    //pokud se nejedná o složku ale o soubor, tak nemůžu smazat
    if(deleting_note->mft_item->isDirectory == 0){
        return -2;
    }

    //neobsahuje potomky => můžu smazat
    if (deleting_note->child == NULL){
        global_bit_map[bitmap_possition] = 0;
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
        printf("SEM TO NIKDY NESMÍ DOJÍT!!\n");
        global_boot_record->number_of_fragments--;
        return 1;
    } else{
        //obsahuje potomky, nelze smazat
        return -1;
    }
}

int delete_node_with_uid_file(int uid_delete){
   struct mft_node *deleting_node = get_node_with_uid(root_directory, uid_delete);
   struct mft_node *deteting_parent = get_node_with_uid(root_directory, deleting_node->mft_item->parent_uid);
   struct mft_node *temp = deteting_parent->child;
   struct mft_node *previous_note = NULL;
   int32_t number_of_clusters;
   int bitmap_start = deleting_node->mft_item->fragments[0].bitmap_start_possition;

   //pokud se nejdedná o soubor, tak nemůžu smazat
   if(deleting_node->mft_item->isDirectory == 1){
       return -1;
   }

    //zjištění potřebný počet clusterů
    if (deleting_node->mft_item->item_size % CLUSTER_SIZE == 0){
        number_of_clusters = (int32_t) deleting_node->mft_item->item_size/CLUSTER_SIZE;
    } else{
        number_of_clusters = (int32_t) (deleting_node->mft_item->item_size/CLUSTER_SIZE) + 1;
    }

    //smazaní všech clusterů, které odpovídají souboru z bitmapy
    for (int i = bitmap_start; i < (bitmap_start + number_of_clusters); i++) {
        global_bit_map[i] = 0;
    }

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

    return -1;

}

/**
 * Přesune soubor s odpovídajícím uid do složky
 * s uid, které je ve druhém parametru této funkce.
 * @param file_uid      uid souboru k přesunutí
 * @param dest_folder   uid složky kam se má přesnout
 * @return  kontrolo -1 = nepovedlo se 1 = povedlo se
 */
int move_node_with_uid(int file_uid, int dest_folder){
    struct mft_node *moving_node = get_node_with_uid(root_directory, file_uid);
    struct mft_node *moving_parent = get_node_with_uid(root_directory, moving_node->mft_item->parent_uid);
    struct mft_node *temp = moving_parent->child;
    struct mft_node *previous_note = NULL;

    //pokud se nejdedná o soubor, tak nemůžu přesouvat
    if(moving_node->mft_item->isDirectory == 1){
        return -1;
    }

    while (temp != NULL){
        if(temp->mft_item->uid == file_uid){
            //přesouvám přímého potomka rodiče
            if(previous_note == NULL){
                moving_parent->child = temp->next;
                add_next_under_uid(root_directory, dest_folder, temp->mft_item);
                temp->mft_item->parent_uid = dest_folder;
                return 1;
            } else{
                //někde v listu
                previous_note->next = temp->next;
                add_next_under_uid(root_directory, dest_folder, temp->mft_item);
                temp->mft_item->parent_uid = dest_folder;
                return 1;
            }
        }
        previous_note = temp;
        temp = temp->next;
    }

    return -1;


}