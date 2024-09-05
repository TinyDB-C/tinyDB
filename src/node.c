#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include "node.h"

node* newEmptyNode(){
    node *newNode = (node*)malloc(sizeof(node));
    if(newNode == NULL){
        printf("Memory allocation failed");
        return NULL;
    }

    //initialize all the values of the struct
    newNode->database = NULL;
    newNode->pageNum = 0;
    newNode->items = NULL;
    newNode->item_count = 0;
    newNode->item_capacity = 0;
    newNode->childNodes = NULL;
    newNode->child_count = 0;
    newNode->child_capacity = 0;

    return newNode;
}

item* createNewItem(const byte *key, size_t key_len, const byte *value, size_t value_len){
    item *newItem = (item*)malloc(sizeof(item));

    //assign values to the struct
    //here, newItem will not have memory ownership of the key and value array
    //if there is some error, allocate new memory for key and value in 'newItem' struct
    newItem->key = key;
    newItem->key_len = key_len;
    newItem->value = value;
    newItem->value_len = value_len;

    return newItem;
}

bool isLeaf(node *n){
    return n->child_count == 0;
}

size_t serialize(node *n, byte *buf, size_t bufSize){
    size_t leftPos = 0;
    size_t rightPos = bufSize;

    //adding page header
    
    //isLeaf
    byte leaf = (byte)isLeaf(n);

    if(leftPos >= bufSize){
        printf("buf overflow 1\n");
        return 0;
    }

    buf[leftPos++] = leaf;

    //key-value pairs count
    uint16_t itemCount = (uint16_t)n->item_count;
    if(leftPos + sizeof(itemCount) > bufSize){
        printf("buf overflow 2\n");
        return 0;
    }
    memcpy(buf+leftPos,&itemCount,sizeof(itemCount));
    leftPos += sizeof(itemCount);

    // We use slotted pages for storing data in the page. It means the actual keys and values (the cells) are appended
	// to right of the page whereas offsets have a fixed size and are appended from the left.
	// It's easier to preserve the logical order (alphabetical in the case of b-tree) using the metadata and performing
	// pointer arithmetic. Using the data itself is harder as it varies by size.

	// Page structure is:
	// ----------------------------------------------------------------------------------
	// |  Page  | key-value /  child node    key-value 		      |    key-value		 |
	// | Header |   offset /	 pointer	  offset         .... |      data      ..... |
	// ----------------------------------------------------------------------------------

    //serialize each item
    for(size_t i=0; i<n->item_count;i++){
        item* it = n->items[i];
        if(!leaf){
            pgnum childNode = n->childNodes[i];
            if(leftPos+sizeof(childNode) > bufSize){
                printf("buf overflow 3\n");
                return 0;
            }
            memcpy(buf+leftPos,&childNode,sizeof(childNode));
            leftPos += sizeof(childNode);
        }

        size_t klen = it->key_len;
        size_t vlen = it->value_len;

        //write offset
        uint16_t offset= (uint16_t)(rightPos - klen - vlen - 2);
        if(leftPos+size(offset) > bufSize){
            printf("buf overflow 4\n");
            return 0;
        }
        memcpy(buf+leftPos,&offset,sizeof(offset));
        leftPos += sizeof(offset);

        //write value
        rightPos -= vlen;
        if(rightPos-1 < leftPos){
            printf("buf overflow 5\n");
            return 0;
        }
        memcpy(buf+rightPos,it->value,vlen);

        //write value length
        buf[--rightPos] = (byte)vlen;

        //write key
        rightPos -= klen;
        if(rightPos-1 < leftPos){
            printf("buf overflow 6\n");
            return 0;
        }
        memcpy(buf+rightPos,it->key,klen);

        //write key length
        buf[--rightPos] = (byte)klen;
    }

    if(!leaf){
        //write the last child node
        pgnum lastchildnode = n->childNodes[(n->child_count)-1];
        memcpy(buf+leftPos,&lastchildnode,sizeof(lastchildnode));
    }

    return leftPos - rightPos + bufSize;
}

void deserialize(node *n, byte *buf, size_t bufSize){
    size_t leftPos = 0;
    
    //read header
    byte leaf = (byte)buf[leftPos++];
    uint16_t itemCount;
    memcpy(&itemCount,buf+leftPos,sizeof(itemCount));

    leftPos+=sizeof(itemCount);

    for(size_t i=0;i<itemCount;i++){
        if(leaf==0){
            uint64_t pageNum;

        }
    }
}