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

    // IDs of all data blocks in use.
    int blocksIDS[];


} pnode;


// Moved this from .c file.
static int   pages_fd   = -1;
static void* pages_base =  0;

// Global variables
// const int SUPER_SIZE = 20;
// const int NUMBER_OF_INODES = 8;
// const int NUMBER_OF_DATABLOCKS = 8;
//
// const int start_iNode_bitMap = SUPER_SIZE;
// const int start_dataBlock_bitMap = start_iNode_bitMap
//                                 + NUMBER_OF_INODES*sizeof(int);
// const int start_iNode_Table = start_dataBlock_bitMap
//                                 + NUMBER_OF_DATABLOCKS*sizeof(int);
// const int start_dataBlocks = start_iNode_Table
//                                 + NUMBER_OF_INODES*sizeof(pnode);
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
void add_node(const char* path, int mode, int size, int xtra, int which_iNode);
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
