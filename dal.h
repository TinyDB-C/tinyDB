#ifndef commonHeader_h
#define commonHeader_h

#include<stdio.h>
#include<stdint.h>

typedef uint64_t pgnum;
typedef unsigned char byte;

typedef struct {
    pgnum num;
    byte *data;
    size_t dataSize;
} page;

typedef struct {
    FILE *file;
    int pageSize;
} dal;

dal* newDal(const char* path, int pageSize);

void closeDal(dal* database);

void closePage(page *p);

page* allocateEmptyPage(dal *database);

page* readPage(dal *database, pgnum pageNum);

int writePage(dal *database, page *p);

#endif