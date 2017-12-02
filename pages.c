
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

  // TODO have a "reset" flush out option?

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
    // Start location of iNode
    SUPER_SIZE = 20;
    NUMBER_OF_INODES = 8;
    NUMBER_OF_DATABLOCKS = 8;

    start_iNode_bitMap = SUPER_SIZE;
    start_dataBlock_bitMap = start_iNode_bitMap
                                    + NUMBER_OF_INODES*sizeof(int);
    start_iNode_Table = start_dataBlock_bitMap
                                    + NUMBER_OF_DATABLOCKS*sizeof(int);
    start_dataBlocks = start_iNode_Table
                                    + NUMBER_OF_INODES*sizeof(pnode);

    // Write offset to start of inode bitmap in the superblock
    write_int_offset(0, start_iNode_bitMap);
    // Write offset to start of data block bitmap in the superblock
    write_int_offset(1, start_dataBlock_bitMap);
    // Write offset to start of inode table in the superblock
    write_int_offset(2, start_iNode_Table);
    // Write offset to start of data blocks in the superblock
    write_int_offset(3, start_dataBlocks);

    // + (int*)(pages_base);

    // TEST
    printf("PAGES BASE: %d\n",pages_base);
    printf("1stOffset:%d\n",start_iNode_bitMap);
    printf("2stOffset:%d\n",start_dataBlock_bitMap);
    printf("3stOffset:%d\n",start_iNode_Table);
    printf("4stOffset:%d\n",start_dataBlocks);

    // Test inserting Inode into thing
    add_node("/",56,25,145,3);
    flip_iNode_bit(3,1);
    printf("Bit On :%d\n",*((int*)(GET_ptr_start_iNode_bitMap() + sizeof(int)*3)));
    printf("%s","Print out the node: xtra should equal 145\n");
    print_node((pnode*)(GET_ptr_start_iNode_Table() + sizeof(pnode)*3));


}

void
write_int_offset(int offset, int data) {
    *((int*)(pages_base + sizeof(int)*offset)) = data;
}


void
write_char_offset(int offset, char data) {
    *((char*)pages_base + sizeof(char)*offset) = data;
}

void
add_node(const char* path,int mode, int size, int xtra, int which_iNode) {
  //TODO add the data block array as an argument of this function and set.

  void* locationToPlace =
            (sizeof(pnode)*which_iNode) + GET_ptr_start_iNode_Table();
  pnode* newNode = (pnode*)(locationToPlace);
  newNode->mode = mode;
  newNode->size = size;
  newNode->xtra = xtra;
  newNode->path = path;

  printf("%s\n", "add node got to this point");
}


// States: 0 == off  // 1 == on
void
flip_iNode_bit(int which_iNode, int state) {

  // Assert that we're changing the state to something useful.
  assert(state == 0 || state == 1);

  void* targetPtr = GET_ptr_start_iNode_bitMap() + sizeof(int)*which_iNode;
  printf("TargetPointer: %d\n", targetPtr);
  //*((int*)(targetPtr)) = 1;
  *((int*)targetPtr) = state;
  //*((int*)targetPtr) = 4;
  printf("Target:%d\n", *((int*)(targetPtr)));
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
        printf("node{refs: %d, mode: %04o, size: %d, xtra: %d, path: %s}\n",
               node->refs, node->mode, node->size, node->xtra, node->path);
    }
    else {
        printf("node{null}\n");
    }
}

// GETTERS
void*
GET_ptr_start_iNode_bitMap() {
  return start_iNode_bitMap + pages_base;
}
void*
GET_ptr_start_dataBlock_bitMap() {
  return start_dataBlock_bitMap + pages_base;
}
void*
GET_ptr_start_iNode_Table() {
  return start_iNode_Table + pages_base;
}
void*
GET_ptr_start_dataBlocks() {
  return start_dataBlocks + pages_base;
}
