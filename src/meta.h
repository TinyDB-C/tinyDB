#ifndef META_H
#define META_H

#include <stdint.h>
#include <stddef.h>

// Define the magic number used for database file identification
#define MAGIC_NUMBER 0x74696e79  // 'HEX code of tiny'

// Type definition for page numbers using a 64-bit unsigned integer
typedef uint64_t pgnum;

// Structure to hold metadata about the database
typedef struct {
    pgnum root;         // Page number where the root collection is stored
    pgnum freelistPage; // Page number where the free list is stored
} Meta;

// Function prototypes

// Creates a new Meta structure with default values (typically zeros)
Meta* new_empty_meta();

// Serializes the Meta structure into a byte buffer for storage or transmission
void serialize_meta(const Meta* m, uint8_t* buf, size_t buf_size);

// Deserializes a byte buffer into a Meta structure
void deserialize_meta(Meta* m, const uint8_t* buf, size_t buf_size);

#endif // META_H
