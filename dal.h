// #ifndef commonHeader_h
// #define commonHeader_h

// #include<stdio.h>
// #include<stdint.h>

// typedef uint64_t pgnum;
// typedef unsigned char byte;

// typedef struct {
//     pgnum num;
//     byte *data;
//     size_t dataSize;
// } page;

// typedef struct {
//     FILE *file;
//     int pageSize;
// } dal;

// dal* newDal(const char* path, int pageSize);

// void closeDal(dal* database);

// void closePage(page *p);

// page* allocateEmptyPage(dal *database);

// page* readPage(dal *database, pgnum pageNum);

// int writePage(dal *database, page *p);

// #endif

// --------------------------------------------------------------- //
// dal.h
#ifndef DAL_H
#define DAL_H

#include <stdint.h>
#include <stdbool.h>
#include "freelist.h"
#include "node2.h"
#include "meta.h"

typedef uint64_t pgnum;

typedef struct {
    int pageSize;
    float minFillPercent;
    float maxFillPercent;
} Options;

extern const Options DefaultOptions;

typedef struct {
    pgnum num;
    uint8_t* data;
} page;


typedef struct {
    int pageSize;
    float minFillPercent;
    float maxFillPercent;
    FILE* file;
    Meta* meta;
    freelist* freelist;
} dal;

// Function prototypes
dal* newDal(const char* path, Options* options);
void closeDal(dal* d);
int getSplitIndex(dal* d, Node* node);
bool isOverPopulated(dal* d, Node* node);
bool isUnderPopulated(dal* d, Node* node);
page* allocateEmptyPage(dal* d);
page* readPage(dal* d, pgnum pageNum);
int writePage(dal* d, page* p);
Node* getNode(dal* d, pgnum pageNum);
Node* writeNode(dal* d, Node* n);
void deleteNode(dal* d, pgnum pageNum);
Meta* readMeta(dal* d);
page* writeMeta(dal* d, Meta* meta);

#endif // DAL_H