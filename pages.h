#ifndef PAGES_H
#define PAGES_H

#include <stdio.h>

typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    int size; // bytes for file
    int xtra; // more stuff can go here

    // IDs of all data blocks in use.
    int blocksIDS[];

} pnode;


// Moved this from .c file.
static int   pages_fd   = -1;
static void* pages_base =  0;


void   pages_init(const char* path);
void   pages_free();
void*  pages_get_page(int pnum);
/*inode**/ pnode* pages_get_node(int node_id);
int    pages_find_empty();
void   print_node(pnode* node);

// Josh: My new methods
void add_node(int mode, int size, int xtra, int which_iNode);

void write_int_offset(int offset, int data);

void write_char_offset(int offset, char data);


#endif
