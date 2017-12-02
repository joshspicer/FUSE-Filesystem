#ifndef PAGES_H
#define PAGES_H

#include <stdio.h>

typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    int size; // bytes for file
    int xtra; // more stuff can go here

    // Path of file
    const char* path;
    // File rename
    char* fileName;

    // IDs of all data blocks in use.
    int blocksIDS[];

} pnode;


// Moved this from .c file.
static int   pages_fd   = -1;
static void* pages_base =  0;

static int SUPER_SIZE;
static int NUMBER_OF_INODES;
static int NUMBER_OF_DATABLOCKS;

static int start_iNode_bitMap;
static int start_dataBlock_bitMap;
static int start_iNode_Table;
static int start_dataBlocks;


void   pages_init(const char* path);
void   pages_free();
void*  pages_get_page(int pnum);
/*inode**/ pnode* pages_get_node(int node_id);
int    pages_find_empty();
void   print_node(pnode* node);

// Josh: My new methods
void add_node(const char* path, char* fileName, int mode, int size, int xtra, int which_iNode);
void flip_iNode_bit(int which_iNode, int state); // 0 == off, 1 == on

void write_int_offset(int offset, int data);
void write_char_offset(int offset, char data);

void* GET_ptr_start_iNode_bitMap();
void* GET_ptr_start_dataBlock_bitMap();
void* GET_ptr_start_iNode_Table();
void* GET_ptr_start_dataBlocks();

int GET_NUMBER_OF_INODES();
int GET_NUMBER_OF_DATABLOCKS();


#endif
