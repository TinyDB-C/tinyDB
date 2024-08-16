#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freelist.h"

const pgnum metaPage = 0;

// Creates a new freelist and initializes it
freelist* newFreelist() {
    freelist* fr = (freelist*)malloc(sizeof(freelist)); // Allocate memory for the freelist
    if (!fr) {
        perror("Failed to allocate memory for freelist");
        exit(EXIT_FAILURE);
    }
    fr->maxPage = metaPage; // Initialize maxPage
    fr->releasedPagesCount = 0; // Initialize releasedPagesCount
    fr->releasedPagesCapacity = 4;  // Initial capacity for the releasedPages array
    fr->releasedPages = (pgnum*)malloc(fr->releasedPagesCapacity * sizeof(pgnum)); // Allocate memory for releasedPages array
    if (!fr->releasedPages) {
        perror("Failed to allocate memory for releasedPages");
        free(fr); // Free previously allocated memory for freelist
        exit(EXIT_FAILURE);
    }
    return fr; // Return the initialized freelist
}

// Frees the memory allocated for the freelist
void freeFreelist(freelist* fr) {
    if (fr) {
        free(fr->releasedPages); // Free the memory for releasedPages array
        free(fr); // Free the memory for the freelist itself
    }
}

// Retrieves the next available page number. If there are no released pages, it increments maxPage
pgnum getNextPage(freelist* fr) {
    if (fr->releasedPagesCount > 0) {
        return fr->releasedPages[--fr->releasedPagesCount]; // Return a released page if available
    }
    return ++fr->maxPage; // Otherwise, return a new page number
}

// Releases a page number back into the freelist
void releasePage(freelist* fr, pgnum page) {
    // Check if we need to resize the releasedPages array
    if (fr->releasedPagesCount == fr->releasedPagesCapacity) {
        fr->releasedPagesCapacity = fr->releasedPagesCapacity * 3 / 2 + 1; // Increase capacity by 1.5x + 1
        pgnum* newBuffer = (pgnum*)realloc(fr->releasedPages, fr->releasedPagesCapacity * sizeof(pgnum)); // Reallocate memory
        if (!newBuffer) {
            perror("Failed to reallocate memory for releasedPages");
            exit(EXIT_FAILURE);
        }
        fr->releasedPages = newBuffer; // Update releasedPages pointer
    }
    fr->releasedPages[fr->releasedPagesCount++] = page; // Add the released page to the list
}

// Serializes the freelist into a byte buffer for storage or transmission
uint8_t* serialize(freelist* fr, size_t* size) {
    // Calculate the required size for serialization
    *size = sizeof(fr->maxPage) + sizeof(fr->releasedPagesCount) + sizeof(pgnum) * fr->releasedPagesCount;
    uint8_t* buf = (uint8_t*)malloc(*size); // Allocate memory for the buffer
    if (!buf) {
        perror("Failed to allocate memory for serialization");
        exit(EXIT_FAILURE);
    }
    size_t pos = 0;

    // Copy data from freelist to the buffer
    memcpy(buf + pos, &fr->maxPage, sizeof(fr->maxPage));
    pos += sizeof(fr->maxPage);

    memcpy(buf + pos, &fr->releasedPagesCount, sizeof(fr->releasedPagesCount));
    pos += sizeof(fr->releasedPagesCount);

    memcpy(buf + pos, fr->releasedPages, fr->releasedPagesCount * sizeof(pgnum));

    return buf; // Return the serialized buffer
}

// Deserializes a byte buffer to recreate a freelist object
freelist* deserialize(uint8_t* buf, size_t size) {
    freelist* fr = newFreelist(); // Create a new freelist instance
    if (!fr) {
        fprintf(stderr, "Failed to allocate memory for freelist\n");
        return NULL;
    }

    // Ensure the buffer is large enough
    if (size < sizeof(fr->maxPage) + sizeof(fr->releasedPagesCount)) {
        fprintf(stderr, "Buffer too small to deserialize freelist\n");
        freeFreelist(fr);
        return NULL;
    }

    size_t pos = 0;

    // Copy data from the buffer to the freelist
    memcpy(&fr->maxPage, buf + pos, sizeof(fr->maxPage));
    pos += sizeof(fr->maxPage);

    memcpy(&fr->releasedPagesCount, buf + pos, sizeof(fr->releasedPagesCount));
    pos += sizeof(fr->releasedPagesCount);

    // Ensure the buffer has enough data for released pages
    if (size < pos + fr->releasedPagesCount * sizeof(pgnum)) {
        fprintf(stderr, "Buffer too small to contain all released pages\n");
        freeFreelist(fr);
        return NULL;
    }

    fr->releasedPagesCapacity = fr->releasedPagesCount > 4 ? fr->releasedPagesCount : 4;
    pgnum* newBuffer = (pgnum*)realloc(fr->releasedPages, fr->releasedPagesCapacity * sizeof(pgnum)); // Allocate memory for releasedPages
    if (!newBuffer) {
        perror("Failed to allocate memory for releasedPages during deserialization");
        freeFreelist(fr);
        return NULL;
    }
    fr->releasedPages = newBuffer;

    memcpy(fr->releasedPages, buf + pos, fr->releasedPagesCount * sizeof(pgnum)); // Copy released pages from buffer

    return fr; // Return the deserialized freelist
}

// Prints the contents of the freelist
void printFreelist(const freelist* fr) {
    if (fr == NULL) {
        printf("Freelist is NULL\n");
        return;
    }
    printf("Freelist:\n");
    printf("  maxPage: %u\n", fr->maxPage); // Print the maxPage
    printf("  releasedPagesCount: %zu\n", fr->releasedPagesCount); // Print the count of released pages
    printf("  releasedPagesCapacity: %zu\n", fr->releasedPagesCapacity); // Print the capacity of released pages array
    printf("  releasedPages: ");
    for (size_t i = 0; i < fr->releasedPagesCount; ++i) {
        printf("%u ", fr->releasedPages[i]); // Print each released page
    }
    printf("\n");
}
