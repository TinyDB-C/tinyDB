#ifndef FREELIST_H
#define FREELIST_H

#include <stdint.h>

// Type definition for page numbers
typedef unsigned int pgnum;

// Structure representing the freelist
typedef struct freelist {
    pgnum maxPage; // The maximum page number allocated so far
    size_t releasedPagesCount; // Number of pages currently in the freelist
    size_t releasedPagesCapacity; // Capacity of the releasedPages array
    pgnum* releasedPages; // Pointer to an array of released pages
} freelist;

// Function prototypes

// Creates and initializes a new freelist
freelist* newFreelist();

// Frees the memory allocated for a freelist
void freeFreelist(freelist* fr);

// Retrieves the next available page number from the freelist
pgnum getNextPage(freelist* fr);

// Releases a page number back into the freelist
void releasePage(freelist* fr, pgnum page);

// Serializes the freelist into a byte buffer for storage or transmission
uint8_t* serialize(freelist* fr, size_t* size);

// Deserializes a byte buffer to recreate a freelist object
freelist* deserialize(uint8_t* buf, size_t size);

// Prints the contents of the freelist for debugging or inspection
void printFreelist(const freelist* fr);

#endif // FREELIST_H
