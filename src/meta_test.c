#include "meta.h"
#include <stdio.h>
#include <stdlib.h>

// Function to print the Meta contents (for debugging)
void print_meta(const Meta* m) {
    printf("Meta - root: %llu, freelistPage: %llu\n", 
           (unsigned long long)m->root, 
           (unsigned long long)m->freelistPage);
}

// Main test function
int main() {
    // Create a new empty Meta
    Meta* meta = new_empty_meta();
    meta->root = 4212111;
    meta->freelistPage = 842321;

    // Print the original Meta
    printf("Original Meta:\n");
    print_meta(meta);

    // Allocate buffer for serialization
    size_t buffer_size = sizeof(uint32_t) + 2 * sizeof(pgnum);
    uint8_t* buffer = (uint8_t*)malloc(buffer_size);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        free(meta);
        exit(EXIT_FAILURE);
    }

    // Serialize the meta structure
    serialize_meta(meta, buffer, buffer_size);

    // Deserialize the meta structure into a new Meta object
    Meta* new_meta = new_empty_meta();
    deserialize_meta(new_meta, buffer, buffer_size);

    // Print the deserialized Meta
    printf("Deserialized Meta:\n");
    print_meta(new_meta);

    // Check if original and deserialized meta are equal
    if (meta->root == new_meta->root && meta->freelistPage == new_meta->freelistPage) {
        printf("Test Passed: Deserialized meta matches the original meta.\n");
    } else {
        printf("Test Failed: Deserialized meta does not match the original meta.\n");
    }

    // Cleanup allocated memory
    free(meta);
    free(new_meta);
    free(buffer);

    return 0;
}
