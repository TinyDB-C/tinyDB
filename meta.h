#ifndef META_H
#define META_H

#include <stdint.h>
#include <stddef.h>

// Define the magic number for the database file
#define MAGIC_NUMBER 0xD00DB00D

// Type definition for page numbers
typedef uint64_t pgnum;

// Structure to hold metadata of the database
typedef struct {
    pgnum root;         // Page number containing the root collection
    pgnum freelistPage; // Page number of the free list
} Meta;

// Function prototypes
Meta* new_empty_meta();
void serialize_meta(const Meta* m, uint8_t* buf, size_t buf_size);
void deserialize_meta(Meta* m, const uint8_t* buf, size_t buf_size);

#endif // META_H
