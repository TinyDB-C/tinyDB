#include<stdio.h>
#include<stdlib.h>
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

dal* newDal(const char* path, int pageSize){
  dal *database = (dal*)malloc(sizeof(dal));
  if(database == NULL){
    return NULL; //memory allocation failed
  }
  
  // open file with read and write access, create if doesn't exist
  database->file = fopen(path, "r+");
  if(database->file == NULL){
    //if file doesn't exist, create it
    database->file = fopen(path, "w+");
    if(database->file == NULL){
      // file opening failed
      // clear database memory and return NULL
      free(database);
      return NULL;
    }
  }

  database->pageSize = pageSize;
  return database;
}

void closeDal(dal* database){
  if(database){
    if(database->file){
      fclose(database->file);
    }
    free(database);
  }
}

void closePage(page *p){
  if(p){
    if(p->data){
      free(p->data);
    }
    free(p);
  }
}

page* allocateEmptyPage(dal *database){
  if(database == NULL){
    return NULL; //invalid pointer
  }

  page *p = (page*)malloc(sizeof(page));
  if(p == NULL){
    return NULL; //memory allocation failed
  }

  p->data = (byte*)malloc(database->pageSize);
  if(p->data == NULL){
    // memory allocation failed
    // clear pae memory and return NULL
    free(p);
    return NULL;
  }

  return p;
}

page* readPage(dal *database, pgnum pageNum){
  if(database ==NULL){
    return NULL; //invalid dal pointer
  }

  page* p = allocateEmptyPage(database);
  if(p == NULL){
    return NULL; //memory allocation failed
  }

  //offset where file read operation will start
  uint64_t offset = pageNum * database->pageSize;

  //move file pointer to offset position
  fseek(database->file, offset, SEEK_SET);
  size_t bytesRead = fread(p->data,1,database->pageSize,database->file);

  //check for errors
  if(bytesRead < database->pageSize){
    closePage(p);
    return NULL;
  }

  return p;
}

int writePage(dal *database, page *p){
  if(database == NULL || p == NULL || p->data == NULL){
    return -1; //return -1 for errors
  }

  uint64_t offset = p->num * database->pageSize;

  if(fseek(database->file, offset, SEEK_SET) != 0) {
        return -1; //error moving file pointer
  }

  size_t bytesWritten = fwrite(p->data, 1, p->dataSize, database->file);

  // Check if the correct number of bytes were written
  if (bytesWritten < p->dataSize) {
      return -1; //error writing data to the file
  }

  return 0; //success
}