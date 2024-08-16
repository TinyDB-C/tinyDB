#ifndef FREELIST_H
#define FREELIST_H

#include <stdint.h>

typedef unsigned int pgnum;

typedef struct freelist {
    pgnum maxPage;
    size_t releasedPagesCount;
    size_t releasedPagesCapacity;
    pgnum* releasedPages;
} freelist;

freelist* newFreelist();
void freeFreelist(freelist* fr);
pgnum getNextPage(freelist* fr);
void releasePage(freelist* fr, pgnum page);
uint8_t* serialize(freelist* fr, size_t* size);
freelist* deserialize(uint8_t* buf, size_t size);
void printFreelist(const freelist* fr); // Add this line

#endif // FREELIST_H
