#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "freelist.h"

// Function prototypes
void testSerializationDeserialization();
void printTestResults(const freelist* original, const freelist* deserialized);

int main() {
    testSerializationDeserialization();
    return 0;
}

void testSerializationDeserialization() {
    // Create a new freelist and add some pages
    freelist* original = newFreelist();
    for (pgnum i = 1; i <= 10; ++i) {
        releasePage(original, i);
    }

    // Print the original freelist
    printf("Original freelist:\n");
    printFreelist(original);

    // Serialize the freelist
    size_t serializedSize;
    uint8_t* serializedData = serialize(original, &serializedSize);

    // Deserialize the freelist
    freelist* deserialized = deserialize(serializedData, serializedSize);

    // Print the deserialized freelist
    printf("Deserialized freelist:\n");
    printFreelist(deserialized);

    // Check if deserialization was successful
    assert(deserialized != NULL);
    assert(deserialized->maxPage == original->maxPage);
    assert(deserialized->releasedPagesCount == original->releasedPagesCount);
    assert(deserialized->releasedPagesCapacity >= original->releasedPagesCount);

    // Verify that released pages are the same
    for (size_t i = 0; i < original->releasedPagesCount; ++i) {
        assert(deserialized->releasedPages[i] == original->releasedPages[i]);
    }

    // Check edge cases
    // Test empty freelist
    freelist* emptyOriginal = newFreelist();
    size_t emptySerializedSize;
    uint8_t* emptySerializedData = serialize(emptyOriginal, &emptySerializedSize);
    freelist* emptyDeserialized = deserialize(emptySerializedData, emptySerializedSize);
    printf("Empty deserialized freelist:\n");
    printFreelist(emptyDeserialized);
    assert(emptyDeserialized != NULL);
    assert(emptyDeserialized->maxPage == emptyOriginal->maxPage);
    assert(emptyDeserialized->releasedPagesCount == 0);
    assert(emptyDeserialized->releasedPagesCapacity >= 4); // Initial capacity

    // Clean up
    freeFreelist(original);
    freeFreelist(deserialized);
    free(serializedData);

    freeFreelist(emptyOriginal);
    free(emptySerializedData);
    free(emptyDeserialized);

    printf("All tests passed!\n");
}
