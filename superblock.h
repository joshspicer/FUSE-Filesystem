#ifndef PAGES_H
#define PAGES_H

#include "node.h"


// Moved this from .c file.
static int pages_fd = -1;
static void *pages_base = 0;

static int SUPER_SIZE;
static int NUMBER_OF_INODES;
static int NUMBER_OF_DATABLOCKS;

static int start_iNode_bitMap;
static int start_dataBlock_bitMap;
static int start_iNode_Table;
static int start_dataBlocks;


void superBlock_init(const char *path);

void write_int_offset(int offset, int data);

void *GET_ptr_start_iNode_bitMap();

void *GET_ptr_start_dataBlock_bitMap();

void *GET_ptr_start_iNode_Table();

void *GET_ptr_start_dataBlocks();

int GET_NUMBER_OF_INODES();

int GET_NUMBER_OF_DATABLOCKS();




#endif
