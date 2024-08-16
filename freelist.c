#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freelist.h"

const pgnum metaPage = 0;

freelist* newFreelist() {
    freelist* fr = (freelist*)malloc(sizeof(freelist));
    if (!fr) {
        perror("Failed to allocate memory for freelist");
        exit(EXIT_FAILURE);
    }
    fr->maxPage = metaPage;
    fr->releasedPagesCount = 0;
    fr->releasedPagesCapacity = 4;  // Initial capacity
    fr->releasedPages = (pgnum*)malloc(fr->releasedPagesCapacity * sizeof(pgnum));
    if (!fr->releasedPages) {
        perror("Failed to allocate memory for releasedPages");
        free(fr);
        exit(EXIT_FAILURE);
    }
    return fr;
}

void freeFreelist(freelist* fr) {
    if (fr) {
        free(fr->releasedPages);
        free(fr);
    }
}

pgnum getNextPage(freelist* fr) {
    if (fr->releasedPagesCount > 0) {
        return fr->releasedPages[--fr->releasedPagesCount];
    }
    return ++fr->maxPage;
}

void releasePage(freelist* fr, pgnum page) {
    if (fr->releasedPagesCount == fr->releasedPagesCapacity) {
        fr->releasedPagesCapacity = fr->releasedPagesCapacity * 3 / 2 + 1;
        pgnum* newBuffer = (pgnum*)realloc(fr->releasedPages, fr->releasedPagesCapacity * sizeof(pgnum));
        if (!newBuffer) {
            perror("Failed to reallocate memory for releasedPages");
            exit(EXIT_FAILURE);
        }
        fr->releasedPages = newBuffer;
    }
    fr->releasedPages[fr->releasedPagesCount++] = page;
}

uint8_t* serialize(freelist* fr, size_t* size) {
    *size = sizeof(fr->maxPage) + sizeof(fr->releasedPagesCount) + sizeof(pgnum) * fr->releasedPagesCount;
    uint8_t* buf = (uint8_t*)malloc(*size);
    if (!buf) {
        perror("Failed to allocate memory for serialization");
        exit(EXIT_FAILURE);
    }
    size_t pos = 0;

    memcpy(buf + pos, &fr->maxPage, sizeof(fr->maxPage));
    pos += sizeof(fr->maxPage);

    memcpy(buf + pos, &fr->releasedPagesCount, sizeof(fr->releasedPagesCount));
    pos += sizeof(fr->releasedPagesCount);

    memcpy(buf + pos, fr->releasedPages, fr->releasedPagesCount * sizeof(pgnum));

    return buf;
}

freelist* deserialize(uint8_t* buf, size_t size) {
    freelist* fr = newFreelist();
    if (!fr) {
        fprintf(stderr, "Failed to allocate memory for freelist\n");
        return NULL;
    }

    if (size < sizeof(fr->maxPage) + sizeof(fr->releasedPagesCount)) {
        fprintf(stderr, "Buffer too small to deserialize freelist\n");
        freeFreelist(fr);
        return NULL;
    }

    size_t pos = 0;

    memcpy(&fr->maxPage, buf + pos, sizeof(fr->maxPage));
    pos += sizeof(fr->maxPage);

    memcpy(&fr->releasedPagesCount, buf + pos, sizeof(fr->releasedPagesCount));
    pos += sizeof(fr->releasedPagesCount);

    if (size < pos + fr->releasedPagesCount * sizeof(pgnum)) {
        fprintf(stderr, "Buffer too small to contain all released pages\n");
        freeFreelist(fr);
        return NULL;
    }

    fr->releasedPagesCapacity = fr->releasedPagesCount > 4 ? fr->releasedPagesCount : 4;
    pgnum* newBuffer = (pgnum*)realloc(fr->releasedPages, fr->releasedPagesCapacity * sizeof(pgnum));
    if (!newBuffer) {
        perror("Failed to allocate memory for releasedPages during deserialization");
        freeFreelist(fr);
        return NULL;
    }
    fr->releasedPages = newBuffer;

    memcpy(fr->releasedPages, buf + pos, fr->releasedPagesCount * sizeof(pgnum));

    return fr;
}

void printFreelist(const freelist* fr) {
    if (fr == NULL) {
        printf("Freelist is NULL\n");
        return;
    }
    printf("Freelist:\n");
    printf("  maxPage: %u\n", fr->maxPage);
    printf("  releasedPagesCount: %zu\n", fr->releasedPagesCount);
    printf("  releasedPagesCapacity: %zu\n", fr->releasedPagesCapacity);
    printf("  releasedPages: ");
    for (size_t i = 0; i < fr->releasedPagesCount; ++i) {
        printf("%u ", fr->releasedPages[i]);
    }
    printf("\n");
}
