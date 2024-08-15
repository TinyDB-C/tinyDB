#ifndef node_h
#define node_h

#include<stdio.h>
#include "dal.h"

typedef unsigned char byte;

typedef struct{
    byte *key;
    byte *value;
    size_t key_len;
    size_t value_len;
} item;

typedef struct{
    dal *database;

    pgnum pageNum;
    item **items;
    size_t item_count;
    pgnum *childNodes;
    size_t child_count;

} node;

node* newEmptyNode();

item* createNewItem(const byte *key, size_t key_len, const byte *value, size_t value_len);

bool isLeaf(node *n);

#endif