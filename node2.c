#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define PAGE_SIZE 4096
#define NODE_HEADER_SIZE 3
#define PAGE_NUM_SIZE 8

typedef struct Item {
    uint8_t *key;
    size_t key_len;
    uint8_t *value;
    size_t value_len;
} Item;

typedef uint64_t pgnum;

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

Node* new_empty_node() {
    Node* node = (Node*)malloc(sizeof(Node));
    node->page_num  = -1;
    node->items = NULL;
    node->items_count = 0;
    node->child_nodes = NULL;
    node->child_nodes_count = 0;
    return node;
}

Node* new_node_for_serialization(Item **items, size_t items_count, pgnum *child_nodes, size_t child_nodes_count) {
    Node* node = new_empty_node();
    node->items = items;
    node->items_count = items_count;
    node->child_nodes = child_nodes;
    node->child_nodes_count = child_nodes_count;
    return node;
}

Item* new_item(uint8_t *key, size_t key_len, uint8_t *value, size_t value_len) {
    //  try item->key = key , same for value too **************************************
    
    Item* item = (Item*)malloc(sizeof(Item));
    item->key = (uint8_t*)malloc(key_len);
    memcpy(item->key, key, key_len);
    item->key_len = key_len;
    item->value = (uint8_t*)malloc(value_len);
    memcpy(item->value, value, value_len);
    item->value_len = value_len;
    return item;
}

bool is_last(int index, Node *parent_node) {
    return index == parent_node->items_count;
}

bool is_first(int index) {
    return index == 0;
}

bool is_leaf(Node *n) {
    return n->child_nodes_count == 0;
}

Node* write_node(Node *n, Node *node) {
    // change this function ***********************************************************
    // This function would typically interact with the transaction (tx) to mark the node as dirty
    // For simplicity, we'll just return the node
    return node;
}

void write_nodes(Node *n, Node **nodes, size_t count) {
    // change this function ***********************************************************
    for (size_t i = 0; i < count; i++) {
        write_node(n, nodes[i]);
    }
}

Node* get_node(Node *n, pgnum page_num) {
    // change this function ***********************************************************
    // This function would typically interact with the transaction (tx) to retrieve a node
    // For simplicity, we'll just return NULL
    return NULL;
}

bool is_over_populated(Node *n) {
    // This would typically check against some maximum size
    // For simplicity, we'll use a fixed threshold
    // change this function ***********************************************************
    return n->items_count > 100;
}

bool can_spare_an_element(Node *n) {
    // This would typically check against some minimum size
    // For simplicity, we'll use a fixed threshold
    // change this function ***********************************************************
    return n->items_count > 2;
}

bool is_under_populated(Node *n) {
    // This would typically check against some minimum size
    // For simplicity, we'll use a fixed threshold
    // change this function ***********************************************************

    return n->items_count < 2;
}

void serialize(Node *n, uint8_t *buf, size_t buf_len) {
    size_t left_pos = 0;
    size_t right_pos =  buf_len - 1;  // check for buf len ************************

    // Add page header: isLeaf, key-value pairs count
    buf[left_pos++] = is_leaf(n) ? 1 : 0;
    *(uint16_t*)(buf + left_pos) = (uint16_t)n->items_count;
    left_pos += 2;

    for (size_t i = 0; i < n->items_count; i++) {
        Item *item = n->items[i];
        if (!is_leaf(n)) {
            *(uint64_t*)(buf + left_pos) = n->child_nodes[i];
            left_pos += sizeof(uint64_t);
        }

        // write offset
        uint16_t offset = (uint16_t)(right_pos - item->key_len - item->value_len - 2);
        *(uint16_t*)(buf + left_pos) = offset;
        left_pos += 2;

        right_pos -= item->value_len;
        memcpy(buf + right_pos, item->value, item->value_len);

        right_pos--;
        buf[right_pos] = (uint8_t)item->value_len;

        right_pos -= item->key_len;
        memcpy(buf + right_pos, item->key, item->key_len);

        right_pos--;
        buf[right_pos] = (uint8_t)item->key_len;
    }

    if (!is_leaf(n)) {
        *(uint64_t*)(buf + left_pos) = n->child_nodes[n->child_nodes_count - 1];
    }

}

void deserialize(Node *n, uint8_t *buf, size_t buf_len) {
    size_t left_pos = 0;

    // Read header
    bool is_leaf = buf[left_pos++] != 0;
    n->items_count = *(uint16_t*)(buf + left_pos);
    left_pos += 2;

    n->items = malloc(n->items_count * sizeof(Item*));
    if (!is_leaf) {
        n->child_nodes = malloc((n->items_count + 1) * sizeof(pgnum));
    }

    // Read body
    for (size_t i = 0; i < n->items_count; i++) {
        if (!is_leaf) {
            n->child_nodes[i] = *(uint64_t*)(buf + left_pos);
            left_pos += PAGE_NUM_SIZE;
        }

        // Read offset
        uint16_t offset = *(uint16_t*)(buf + left_pos);
        left_pos += 2;

        uint8_t klen = buf[offset++];
        uint8_t *key = buf + offset;
        offset += klen;

        uint8_t vlen = buf[offset++];
        uint8_t *value = buf + offset;

        n->items[i] = new_item(key, klen, value, vlen);
    }

    if (!is_leaf) {
        n->child_nodes[n->items_count] = *(uint64_t*)(buf + left_pos);
        n->child_nodes_count = n->items_count + 1;
    }
}

size_t element_size(Node *n, int i) {
    size_t size = 0;
    size += n->items[i]->key_len;
    size += n->items[i]->value_len;
    size += sizeof(pgnum); // 8 is the pgnum size
    return size;
}

size_t node_size(Node *n) {
    size_t size = NODE_HEADER_SIZE;
    for (size_t i = 0; i < n->items_count; i++) {
        size += element_size(n, i);
    }
    size += sizeof(pgnum); // Last child pointer
    return size;
}

int find_key(Node *n, uint8_t *key, size_t key_len, bool exact, Node *result_node, int *ancestors_indexes, size_t *ancestors_count) {
    int index;
    bool found = find_key_in_node(n, key, key_len, &index);
    if (found) {
        result_node = n;
        return index;
    }

    if (is_leaf(n)) {
        if (exact) {
            return -1;
        }
        result_node = n;
        return index;
    }
    // use ancestor capacity to get less realloc *********************************************************
    // Allocate memory for ancestors if needed
    if (ancestors_indexes == NULL) {
        ancestors_indexes = malloc(sizeof(int));
        *ancestors_count = 0;
    } else {
        ancestors_indexes = realloc(ancestors_indexes, (*ancestors_count + 1) * sizeof(int));
    }
    ancestors_indexes[(*ancestors_count)++] = index;

    Node *next_child = get_node(n, n->child_nodes[index]);
    return find_key(next_child, key, key_len, exact, result_node, ancestors_indexes, ancestors_count);
}

bool find_key_in_node(Node *n, uint8_t *key, size_t key_len, int *index) {
    for (int i = 0; i < n->items_count; i++) {
        int cmp = memcmp(n->items[i]->key, key, key_len < n->items[i]->key_len ? key_len : n->items[i]->key_len);
        if (cmp == 0) {
            if (key_len == n->items[i]->key_len) {
                *index = i;
                return true;
            }
            cmp = key_len - n->items[i]->key_len;
        }
        if (cmp < 0) {
            *index = i;
            return false;
        }
    *index = n->items_count;
    }

    return false;
}
int add_item(Node *n, Item *item, int insertion_index) {
    // this function need to be checked ************************************************************
    if (insertion_index == n->items_count) {
        n->items = realloc(n->items, (n->items_count + 1) * sizeof(Item*));
        n->items[n->items_count] = item;
        n->items_count++;
    } else {
        n->items = realloc(n->items, (n->items_count + 1) * sizeof(Item*));
        memmove(&n->items[insertion_index + 1], &n->items[insertion_index], (n->items_count - insertion_index) * sizeof(Item*));
        n->items[insertion_index] = item;
        n->items_count++;
    }
    return insertion_index;
}

void split(Node *n, Node *node_to_split, int node_to_split_index) {
    // this function need to be checked ************************************************************
    // This is a complex operation that would require interaction with the database
    // For simplicity, we'll just implement a basic split
    int split_index = node_to_split->items_count / 2;
    
    Item *middle_item = node_to_split->items[split_index];
    Node *new_node = new_empty_node();

    // Move items to the new node
    new_node->items = malloc((node_to_split->items_count - split_index - 1) * sizeof(Item*));
    memcpy(new_node->items, &node_to_split->items[split_index + 1], (node_to_split->items_count - split_index - 1) * sizeof(Item*));
    new_node->items_count = node_to_split->items_count - split_index - 1;

    // Update the original node
    node_to_split->items_count = split_index;

    if (!is_leaf(node_to_split)) {
        new_node->child_nodes = malloc((node_to_split->child_nodes_count - split_index - 1) * sizeof(pgnum));
        memcpy(new_node->child_nodes, &node_to_split->child_nodes[split_index + 1], (node_to_split->child_nodes_count - split_index - 1) * sizeof(pgnum));
        new_node->child_nodes_count = node_to_split->child_nodes_count - split_index - 1;

        node_to_split->child_nodes_count = split_index + 1;
    }

    // Add the middle item to the parent
    add_item(n, middle_item, node_to_split_index);

    // Add the new node to the parent's children
    n->child_nodes = realloc(n->child_nodes, (n->child_nodes_count + 1) * sizeof(pgnum));
    memmove(&n->child_nodes[node_to_split_index + 2], &n->child_nodes[node_to_split_index + 1], (n->child_nodes_count - node_to_split_index - 1) * sizeof(pgnum));
    n->child_nodes[node_to_split_index + 1] = new_node->page_num;
    n->child_nodes_count++;

    // Write changes
    write_nodes(n, (Node*[]){n, node_to_split, new_node}, 3);
}

void rebalance_remove(Node *n, Node *unbalanced_node, int unbalanced_node_index) {
    // This is a complex operation that would require interaction with the database
    // For simplicity, we'll just implement a basic rebalance
    if (unbalanced_node_index > 0) {
        Node *left_sibling = get_node(n, n->child_nodes[unbalanced_node_index - 1]);
        if (left_sibling->items_count > 1) {
            rotate_right(left_sibling, n, unbalanced_node, unbalanced_node_index);
            write_nodes(n, (Node*[]){left_sibling, n, unbalanced_node}, 3);
            return;
        }
    }

    if (unbalanced_node_index < n->child_nodes_count - 1) {
        Node *right_sibling = get_node(n, n->child_nodes[unbalanced_node_index + 1]);
        if (right_sibling->items_count > 1) {
            rotate_left(unbalanced_node, n, right_sibling, unbalanced_node_index);
            write_nodes(n, (Node*[]){unbalanced_node, n, right_sibling}, 3);
            return;
        }
    }

    // If we can't rotate, we need to merge
    if (unbalanced_node_index > 0) {
        merge(n, unbalanced_node, unbalanced_node_index);
    } else {
        Node *right_sibling = get_node(n, n->child_nodes[1]);
        merge(n, right_sibling, 1);
    }
}

void remove_item_from_leaf(Node *n, int index) {
    memmove(&n->items[index], &n->items[index + 1], (n->items_count - index - 1) * sizeof(Item*));
    n->items_count--;
    write_node(n, n);
}

int* remove_item_from_internal(Node *n, int index, int *affected_nodes_count) {
    int *affected_nodes = malloc(sizeof(int));
    *affected_nodes_count = 1;
    affected_nodes[0] = index;

    Node *predecessor = get_node(n, n->child_nodes[index]);
    while (!is_leaf(predecessor)) {
        int traversing_index = predecessor->child_nodes_count - 1;
        affected_nodes = realloc(affected_nodes, (*affected_nodes_count + 1) * sizeof(int));
        affected_nodes[*affected_nodes_count] = traversing_index;
        (*affected_nodes_count)++;
        predecessor = get_node(predecessor, predecessor->child_nodes[traversing_index]);
    }

    // Replace the item that should be removed with the last item of the predecessor
    free(n->items[index]);
    n->items[index] = predecessor->items[predecessor->items_count - 1];
    predecessor->items_count--;

    write_node(n, n);
    write_node(n, predecessor);

    return affected_nodes;
}

void rotate_right(Node *a_node, Node *p_node, Node *b_node, int b_node_index) {
    // Get last item from a_node and remove it
    Item *a_node_item = a_node->items[a_node->items_count - 1];
    a_node->items_count--;

    // Get item from parent node and assign the a_node_item instead
    int p_node_item_index = b_node_index - 1;
    if (is_first(b_node_index)) {
        p_node_item_index = 0;
    }
    Item *p_node_item = p_node->items[p_node_item_index];
    p_node->items[p_node_item_index] = a_node_item;

    // Assign parent item to b and make it first
    b_node->items = realloc(b_node->items, (b_node->items_count + 1) * sizeof(Item*));
    memmove(&b_node->items[1], b_node->items, b_node->items_count * sizeof(Item*));
    b_node->items[0] = p_node_item;
    b_node->items_count++;

    // If it's an inner node then move children as well
    if (!is_leaf(a_node)) {
        pgnum child_node_to_shift = a_node->child_nodes[a_node->child_nodes_count - 1];
        a_node->child_nodes_count--;

        b_node->child_nodes = realloc(b_node->child_nodes, (b_node->child_nodes_count + 1) * sizeof(pgnum));
        memmove(&b_node->child_nodes[1], b_node->child_nodes, b_node->child_nodes_count * sizeof(pgnum));
        b_node->child_nodes[0] = child_node_to_shift;
        b_node->child_nodes_count++;
    }
}

void rotate_left(Node *a_node, Node *p_node, Node *b_node, int b_node_index) {
    // Get first item from b_node and remove it
    Item *b_node_item = b_node->items[0];
    memmove(b_node->items, &b_node->items[1], (b_node->items_count - 1) * sizeof(Item*));
    b_node->items_count--;

    // Get item from parent node and assign the b_node_item instead
    int p_node_item_index = b_node_index;
    if (is_last(b_node_index, p_node)) {
        p_node_item_index = p_node->items_count - 1;
    }
    Item *p_node_item = p_node->items[p_node_item_index];
    p_node->items[p_node_item_index] = b_node_item;

    // Assign parent item to a and make it last
    a_node->items = realloc(a_node->items, (a_node->items_count + 1) * sizeof(Item*));
    a_node->items[a_node->items_count] = p_node_item;
    a_node->items_count++;

    // If it's an inner node then move children as well
    if (!is_leaf(b_node)) {
        pgnum child_node_to_shift = b_node->child_nodes[0];
        memmove(b_node->child_nodes, &b_node->child_nodes[1], (b_node->child_nodes_count - 1) * sizeof(pgnum));
        b_node->child_nodes_count--;

        a_node->child_nodes = realloc(a_node->child_nodes, (a_node->child_nodes_count + 1) * sizeof(pgnum));
        a_node->child_nodes[a_node->child_nodes_count] = child_node_to_shift;
        a_node->child_nodes_count++;
    }
}

void merge(Node *n, Node *b_node, int b_node_index) {
    Node *a_node = get_node(n, n->child_nodes[b_node_index - 1]);

    // Take the item from the parent, remove it and add it to the unbalanced node
    Item *p_node_item = n->items[b_node_index - 1];
    memmove(&n->items[b_node_index - 1], &n->items[b_node_index], (n->items_count - b_node_index) * sizeof(Item*));
    n->items_count--;

    a_node->items = realloc(a_node->items, (a_node->items_count + 1) * sizeof(Item*));
    a_node->items[a_node->items_count] = p_node_item;
    a_node->items_count++;

    // Merge b_node items into a_node
    a_node->items = realloc(a_node->items, (a_node->items_count + b_node->items_count) * sizeof(Item*));
    memcpy(&a_node->items[a_node->items_count], b_node->items, b_node->items_count * sizeof(Item*));
    a_node->items_count += b_node->items_count;

    // Remove b_node from parent's child nodes
    memmove(&n->child_nodes[b_node_index], &n->child_nodes[b_node_index + 1], (n->child_nodes_count - b_node_index - 1) * sizeof(pgnum));
    n->child_nodes_count--;

    if (!is_leaf(a_node)) {
        // Merge b_node child nodes into a_node
        a_node->child_nodes = realloc(a_node->child_nodes, (a_node->child_nodes_count + b_node->child_nodes_count) * sizeof(pgnum));
        memcpy(&a_node->child_nodes[a_node->child_nodes_count], b_node->child_nodes, b_node->child_nodes_count * sizeof(pgnum));
        a_node->child_nodes_count += b_node->child_nodes_count;
    }

    write_nodes(n, (Node*[]){a_node, n}, 2);
    // Note: In a real implementation, you would need to handle freeing the memory for b_node and updating any relevant data structures
}

