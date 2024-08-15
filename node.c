#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include "node.h"

node* newEmptyNode(){
    node *newNode = (node*)malloc(sizeof(node));
    if(newNode == NULL){
        perror("Memory allocation failed");
        return NULL;
    }

    //initialize all the values of the struct
    newNode->database = NULL;
    newNode->pageNum = 0;
    newNode->items = NULL;
    newNode->item_count = 0;
    newNode->childNodes = NULL;
    newNode->child_count = 0;

    return newNode;
}

item* createNewItem(const byte *key, size_t key_len, const byte *value, size_t value_len){
    item *newItem = (item*)malloc(sizeof(item));

    //assign values to the struct
    //here, newItem will not have memory ownership of the key and value array
    //if there is some error, allocate new memory for key and value in 'newItem' struct
    newItem->key = key;
    newItem->key_len = key_len;
    newItem->value = value;
    newItem->value_len = value_len;

    return newItem;
}

bool isLeaf(node *n){
    return n->child_count == 0;
}