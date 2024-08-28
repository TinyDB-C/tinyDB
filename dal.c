// #include<stdio.h>
// #include<stdlib.h>
// #include "dal.h"

// dal* newDal(const char* path, int pageSize){
//     dal *database = (dal*)malloc(sizeof(dal));
//     if(database == NULL){
//         return NULL; //memory allocation failed
//     }
  
//     // open file with read and write access, create if doesn't exist
//     database->file = fopen(path, "r+");
//     if(database->file == NULL){
//         //if file doesn't exist, create it
//         database->file = fopen(path, "w+");
//         if(database->file == NULL){
//         // file opening failed
//         // clear database memory and return NULL
//         free(database);
//         return NULL;
//         }
//     }

//     database->pageSize = pageSize;
//     return database;
// }

// void closeDal(dal* database){
//     if(database){
//         if(database->file){
//         fclose(database->file);
//         }
//         free(database);
//     }
// }

// void closePage(page *p){
//     if(p){
//         if(p->data){
//         free(p->data);
//         }
//         free(p);
//     }
// }

// page* allocateEmptyPage(dal *database){
//     if(database == NULL){
//         return NULL; //invalid pointer
//     }

//     page *p = (page*)malloc(sizeof(page));
//     if(p == NULL){
//         return NULL; //memory allocation failed
//     }

//     p->data = (byte*)malloc(database->pageSize);
//     if(p->data == NULL){
//         // memory allocation failed
//         // clear pae memory and return NULL
//         free(p);
//         return NULL;
//     }

//     return p;
// }

// page* readPage(dal *database, pgnum page_num){
//     if(database ==NULL){
//         return NULL; //invalid dal pointer
//     }

//     page* p = allocateEmptyPage(database);
//     if(p == NULL){
//         return NULL; //memory allocation failed
//     }

//     //offset where file read operation will start
//     uint64_t offset = page_num * database->pageSize;

//     //move file pointer to offset position
//     fseek(database->file, offset, SEEK_SET);
//     size_t bytesRead = fread(p->data,1,database->pageSize,database->file);

//     //check for errors
//     if(bytesRead < database->pageSize){
//         closePage(p);
//         return NULL;
//     }

//     return p;
// }

// int writePage(dal *database, page *p){
//     if(database == NULL || p == NULL || p->data == NULL){
//         return -1; //return -1 for errors
//     }

//     uint64_t offset = p->num * database->pageSize;

//     if(fseek(database->file, offset, SEEK_SET) != 0) {
//             return -1; //error moving file pointer
//     }

//     size_t bytesWritten = fwrite(p->data, 1, p->dataSize, database->file);

//     // Check if the correct number of bytes were written
//     if (bytesWritten < p->dataSize) {
//         return -1; //error writing data to the file
//     }

//     return 0; //success
// }

// dal.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "dal.h"
#include "node.h"

// Default options for the Disk Abstraction Layer (DAL)
const Options DefaultOptions = {
    .minFillPercent = 0.5f,
    .maxFillPercent = 0.95f
};

// Create a new DAL instance or open an existing one
dal* newDal(const char* path, Options* options) {
    dal* d = (dal*)malloc(sizeof(dal)); // Allocate memory for DAL structure
    if (!d) {
        perror("Failed to allocate memory for dal");
        return NULL;
    }

    // Initialize DAL with options
    d->pageSize = options->pageSize;
    d->minFillPercent = options->minFillPercent;
    d->maxFillPercent = options->maxFillPercent;

    // Try to open the file; if it doesn't exist, create it
    d->file = fopen(path, "r+b");
    if (!d->file) {
        if (errno == ENOENT) {
            // File doesn't exist, create a new one
            d->file = fopen(path, "w+b");
            if (!d->file) {
                perror("Failed to create file");
                free(d);
                return NULL;
            }

            // Initialize freelist and meta information
            d->freelist = newFreelist();
            d->meta = (meta*)malloc(sizeof(meta));
            if (!d->meta) {
                perror("Failed to allocate memory for meta");
                fclose(d->file);
                freeFreelist(d->freelist);
                free(d);
                return NULL;
            }
            d->meta->freelistPage = getNextPage(d->freelist);

            // Write initial freelist to the file
            page* freelistPage = writeMeta(d, d->meta);
            if (!freelistPage) {
                fclose(d->file);
                freeFreelist(d->freelist);
                free(d->meta);
                free(d);
                return NULL;
            }

            // Initialize root node
            Node* collectionsNode = NewNodeForSerialization(NULL, 0, NULL, 0);
            Node* writtenNode = writeNode(d, collectionsNode);
            if (!writtenNode) {
                fclose(d->file);
                freeFreelist(d->freelist);
                free(d->meta);
                free(d);
                return NULL;
            }
            d->meta->root = writtenNode->page_num;

            // Write meta information to the file
            if (!writeMeta(d, d->meta)) {
                fclose(d->file);
                freeFreelist(d->freelist);
                free(d->meta);
                free(d);
                return NULL;
            }
        } else {
            perror("Failed to open file");
            free(d);
            return NULL;
        }
    } else {
        // File exists, read meta and freelist information
        d->meta = readMeta(d);
        if (!d->meta) {
            fclose(d->file);
            free(d);
            return NULL;
        }

        d->freelist = deserialize(readPage(d, d->meta->freelistPage)->data, d->pageSize);
        if (!d->freelist) {
            fclose(d->file);
            free(d->meta);
            free(d);
            return NULL;
        }
    }

    return d;
}

// Close the DAL instance and clean up resources
void closeDal(dal* d) {
    if (d) {
        if (d->file) {
            fclose(d->file);
        }
        if (d->meta) {
            free(d->meta);
        }
        if (d->freelist) {
            freeFreelist(d->freelist);
        }
        free(d);
    }
}

// Determine the index at which to split a node
int getSplitIndex(dal* d, Node* node) {
    int size = NODE_HEADER_SIZE;

    // Iterate through node items to find split index
    for (int i = 0; i < node->items_count; i++) {
        size += elementSize(node, i);

        // Check if the node size exceeds min fill percent and is not the last item
        if ((float)size > d->minFillPercent * (float)d->pageSize && i < node->items_count - 1) {
            return i + 1;
        }
    }

    return -1; // No split index found
}

// Check if a node is over-populated based on max fill percent
bool isOverPopulated(dal* d, Node* node) {
    return (float)nodeSize(node) > d->maxFillPercent * (float)d->pageSize;
}

// Check if a node is under-populated based on min fill percent
bool isUnderPopulated(dal* d, Node* node) {
    return (float)nodeSize(node) < d->minFillPercent * (float)d->pageSize;
}

// Allocate and initialize an empty page
page* allocateEmptyPage(dal* d) {
    page* p = (page*)malloc(sizeof(page)); // Allocate memory for page structure
    if (!p) {
        perror("Failed to allocate memory for page");
        return NULL;
    }
    p->data = (uint8_t*)malloc(d->pageSize); // Allocate memory for page data
    if (!p->data) {
        perror("Failed to allocate memory for page data");
        free(p);
        return NULL;
    }
    memset(p->data, 0, d->pageSize); // Initialize page data to zero
    return p;
}

// Read a page from the file given its page number
page* readPage(dal* d, pgnum page_num) {
    page* p = allocateEmptyPage(d); // Allocate memory for the page
    if (!p) return NULL;

    long offset = (long)page_num * d->pageSize;
    if (fseek(d->file, offset, SEEK_SET) != 0) {
        perror("Failed to seek in file");
        free(p->data);
        free(p);
        return NULL;
    }

    size_t bytesRead = fread(p->data, 1, d->pageSize, d->file);
    if (bytesRead != d->pageSize) {
        if (feof(d->file)) {
            fprintf(stderr, "Unexpected end of file\n");
        } else {
            perror("Failed to read page");
        }
        free(p->data);
        free(p);
        return NULL;
    }

    p->num = page_num;
    return p;
}

// Write a page to the file
int writePage(dal* d, page* p) {
    long offset = (long)p->num * d->pageSize;
    if (fseek(d->file, offset, SEEK_SET) != 0) {
        perror("Failed to seek in file");
        return -1;
    }

    size_t bytesWritten = fwrite(p->data, 1, d->pageSize, d->file);
    if (bytesWritten != d->pageSize) {
        perror("Failed to write page");
        return -1;
    }

    return 0;
}

// Retrieve a node from a given page number
Node* getNode(dal* d, pgnum page_num) {
    page* p = readPage(d, page_num); // Read page from file
    if (!p) return NULL;

    Node* node = NewEmptyNode(); // Create a new node
    if (!node) {
        free(p->data);
        free(p);
        return NULL;
    }

    deserializeNode(node, p->data); // Deserialize node data from page
    node->page_num = page_num;

    free(p->data);
    free(p);
    return node;
}

// Write a node to a page and update its page number
Node* writeNode(dal* d, Node* n) {
    page* p = allocateEmptyPage(d); // Allocate memory for page
    if (!p) return NULL;

    if (n->page_num == 0) {
        // If node has no page number, allocate a new page from freelist
        p->num = getNextPage(d->freelist);
        n->page_num = p->num;
    } else {
        // Otherwise, use the existing page number
        p->num = n->page_num;
    }

    serializeNode(n, p->data); // Serialize node data into page

    if (writePage(d, p) != 0) {
        free(p->data);
        free(p);
        return NULL;
    }

    free(p->data);
    free(p);
    return n;
}

// Release a page number back to the freelist
void deleteNode(dal* d, pgnum page_num) {
    releasePage(d->freelist, page_num);
}

// Read meta information from the meta page
meta* readMeta(dal* d) {
    page* p = readPage(d, metaPage); // Read meta page
    if (!p) return NULL;

    meta* m = (meta*)malloc(sizeof(meta)); // Allocate memory for meta
    if (!m) {
        perror("Failed to allocate memory for meta");
        free(p->data);
        free(p);
        return NULL;
    }

    memcpy(m, p->data, sizeof(meta)); // Copy meta data from page

    free(p->data);
    free(p);
    return m;
}

// Write meta information to the meta page
page* writeMeta(dal* d, meta* meta) {
    page* p = allocateEmptyPage(d); // Allocate memory for page
    if (!p) return NULL;

    p->num = metaPage;
    memcpy(p->data, meta, sizeof(meta)); // Copy meta data to page

    if (writePage(d, p) != 0) {
        free(p->data);
        free(p);
        return NULL;
    }

    return p;
}
