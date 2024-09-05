#include "meta.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Function to create a new, empty Meta structure
Meta* new_empty_meta() {
    Meta* m = (Meta*)malloc(sizeof(Meta)); // Allocate memory for Meta
    if (!m) {
        fprintf(stderr, "Memory allocation failed\n"); // Error message for allocation failure
        exit(EXIT_FAILURE); // Exit the program on memory allocation failure
    }
    m->root = 0;         // Initialize root page number to 0
    m->freelistPage = 0; // Initialize freelist page number to 0
    return m;
}

// Function to serialize the Meta structure into a buffer
void serialize_meta(const Meta* m, uint8_t* buf, size_t buf_size) {
    if (buf_size < sizeof(uint32_t) + 2 * sizeof(pgnum)) {
        fprintf(stderr, "Buffer size is too small for serialization\n"); // Error message for insufficient buffer size
        exit(EXIT_FAILURE); // Exit the program if the buffer size is insufficient
    }

    size_t pos = 0;
    uint32_t magic = MAGIC_NUMBER;
    memcpy(buf + pos, &magic, sizeof(magic)); // Copy magic number to buffer
    pos += sizeof(magic);

    memcpy(buf + pos, &(m->root), sizeof(m->root)); // Copy root page number to buffer
    pos += sizeof(m->root);

    memcpy(buf + pos, &(m->freelistPage), sizeof(m->freelistPage)); // Copy freelist page number to buffer
}

// Function to deserialize the Meta structure from a buffer
void deserialize_meta(Meta* m, const uint8_t* buf, size_t buf_size) {
    if (buf_size < sizeof(uint32_t) + 2 * sizeof(pgnum)) {
        fprintf(stderr, "Buffer size is too small for deserialization\n"); // Error message for insufficient buffer size
        exit(EXIT_FAILURE); // Exit the program if the buffer size is insufficient
    }

    size_t pos = 0;
    uint32_t magicNumberRes;
    memcpy(&magicNumberRes, buf + pos, sizeof(magicNumberRes)); // Copy magic number from buffer
    pos += sizeof(magicNumberRes);

    if (magicNumberRes != MAGIC_NUMBER) {
        fprintf(stderr, "The file is not a valid db file\n"); // Error message for invalid magic number
        exit(EXIT_FAILURE); // Exit the program if the magic number is incorrect
    }

    memcpy(&(m->root), buf + pos, sizeof(m->root)); // Copy root page number from buffer
    pos += sizeof(m->root);

    memcpy(&(m->freelistPage), buf + pos, sizeof(m->freelistPage)); // Copy freelist page number from buffer
}
