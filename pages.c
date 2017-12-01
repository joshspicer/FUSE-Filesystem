
#define _GNU_SOURCE
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include "pages.h"
#include "slist.h"
#include "util.h"

const int NUFS_SIZE  = 1024 * 1024; // 1MB
const int PAGE_COUNT = 256;

void
pages_init(const char* path)
{

    printf("%s\n","PAGE INIT STARTING.");

    pages_fd = open(path, O_CREAT | O_RDWR, 0644);
    assert(pages_fd != -1);

    int rv = ftruncate(pages_fd, NUFS_SIZE);
    assert(rv == 0);

    pages_base = mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, pages_fd, 0);
    assert(pages_base != MAP_FAILED);

    printf("IN PAGES FD:%d\n", pages_fd);

    //*((char*)pages_base) = 'H';

    // --- CREATE SUPERBLOCK ASSIGNING OFFSETS ---
    // Start location of iNode Bitmap
    write_int_offset(0,10);
    start_iNode_bitMap = (int*)(pages_base);
    // Start location of Data block bitmap
    write_int_offset(1,10+sizeof(int)*10);
    start_dataBlock_bitMap = (int*)(pages_base+sizeof(int)*1);
    // Start of iNode Table
    write_int_offset(2,10+sizeof(int)*20);
    start_iNode_Table = (int*)(pages_base+sizeof(int)*2);
    // Data blocks
    write_int_offset(3,10+sizeof(int)*20+sizeof(pnode)*10);
    start_dataBlocks = (int*)(pages_base+sizeof(int)*3);


    printf("PAGES BASE: %d\n",pages_base);
    printf("1stOffset Mem Address:%d\n",start_iNode_bitMap);
    printf("2stOffset Mem Address:%d\n",start_dataBlock_bitMap);
    printf("3stOffset Mem Address:%d\n",start_iNode_Table);
    printf("4stOffset Mem Address:%d\n",start_dataBlocks);

    printf("1stOffset Offset Location: %d\n",*((int*)start_iNode_bitMap));
    printf("2stOffset Offset Location: %d\n",*((int*)start_dataBlock_bitMap));
    printf("3stOffset Offset Location:%d\n",*((int*)start_iNode_Table));
    printf("4stOffset Offset Location:%d\n",*((int*)start_dataBlocks));

    // Test inserting Inode into thing
    add_node(56,25,145,3);
    printf("%s","Print out the node: xtra should equal 145\n");
    print_node((pnode*)(start_iNode_Table + sizeof(pnode)*3));

}

void write_int_offset(int offset, int data) {
    *((int*)(pages_base + sizeof(int)*offset)) = data;
}


void write_char_offset(int offset, char data) {
    *((char*)pages_base + sizeof(char)*offset) = data;
}

void
add_node(int mode, int size, int xtra, int which_iNode) {
  //TODO add the data block array as an argument of this function and set.

  void* locationToPlace = (sizeof(pnode)*which_iNode) + start_iNode_Table;
  pnode* newNode = (pnode*)(locationToPlace);
  newNode->mode = mode;
  newNode->size = size;
  newNode->xtra = xtra;
}

void
pages_free()
{
    int rv = munmap(pages_base, NUFS_SIZE);
    assert(rv == 0);
}

void*
pages_get_page(int pnum)
{
    return pages_base + 4096 * pnum;
}

pnode*
pages_get_node(int nodeNum)  //NOTE: changed node_id to nodeNum
{
    pnode* idx = (pnode*) pages_get_page(0);
    return &(idx[nodeNum]);
}

int
pages_find_empty()
{
    int pnum = -1;
    for (int ii = 2; ii < PAGE_COUNT; ++ii) {
        if (0) { // if page is empty
            pnum = ii;
            break;
        }
    }
    return pnum;
}

void
print_node(pnode* node)
{
    if (node) {
        printf("node{refs: %d, mode: %04o, size: %d, xtra: %d}\n",
               node->refs, node->mode, node->size, node->xtra);
    }
    else {
        printf("node{null}\n");
    }
}

// GETTERS
int GET_OFFSET_start_iNode_bitMap() {
  return *((int*)start_iNode_bitMap);
}
int GET_OFFSET_start_dataBlock_bitMap() {
  return *((int*)start_dataBlock_bitMap);
}
int GET_OFFSET_start_iNode_Table() {
  return *((int*)start_iNode_Table);
}
int GET_OFFSET_start_dataBlocks() {
  return *((int*)start_dataBlocks);
}
