#ifndef node2_h
#define node2_h

#include<stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define PAGE_SIZE 4096
#define NODE_HEADER_SIZE 3
#define PAGE_NUM_SIZE 8

typedef uint64_t pgnum;

typedef struct Item {
    uint8_t *key;
    size_t key_len;
    uint8_t *value;
    size_t value_len;
} Item;

typedef struct Node {
    struct tx *tx;
    pgnum page_num;
    Item **items;
    size_t items_count;
    pgnum *child_nodes;
    size_t child_nodes_count;
} Node;

// Forward declaration of tx struct
struct tx;

Node* new_empty_node();

Node* new_node_for_serialization(Item **items, size_t items_count, pgnum *child_nodes, size_t child_nodes_count);

Item* new_item(uint8_t *key, size_t key_len, uint8_t *value, size_t value_len);

bool is_last(int index, Node *parent_node);

bool is_first(int index);

bool is_leaf(Node *n);

#endif //node2_h