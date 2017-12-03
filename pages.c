
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
#include "storage.h"
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

    // TEST
    printf("PAGES BASE: %p\n",pages_base);
    printf("1stOffset: %d\n",start_iNode_bitMap);
    printf("2stOffset: %d\n",start_dataBlock_bitMap);
    printf("3stOffset: %d\n",start_iNode_Table);
    printf("4stOffset: %d\n",start_dataBlocks);

    // Init the root directory.
    add_node("/", 040755,25,145,0);
    flip_iNode_bit(0,1);
    print_node((pnode*)(GET_ptr_start_iNode_Table() + sizeof(pnode)*0));
          //TODO fix the weird bug where commenting out "addnode" above
          //      causes the last part of this print node to print garbage

   // add_node("/maddie.txt",S_IFREG,50,166,1);
   // flip_iNode_bit(1,1);
   //
   // add_node("/joshua.txt",S_IFREG | S_IRWXU,20,166,2);
   // flip_iNode_bit(2,1);

}

void
write_int_offset(int offset, int data) {
    *((int*)(pages_base + sizeof(int)*offset)) = data;
}


// void
// write_char_offset(int offset, char data) {
//     *((char*)pages_base + sizeof(char)*offset) = data;
// }

void
add_node(const char* completePath, int mode, int size, int xtra, int which_iNode) {
  //TODO add the data block array as an argument of this function and set.

  void* locationToPlace =
            (sizeof(pnode)*which_iNode) + GET_ptr_start_iNode_Table();

  pnode* newNode = (pnode*)(locationToPlace);
  newNode->mode = mode;
  newNode->size = size;
  newNode->xtra = xtra;


  //newNode->path = *(completePath);
  for (int i = 0; i < strlen(completePath); i++) {
    newNode->path[i] = completePath[i];
  }
  newNode->path[strlen(completePath)] = NULL;

  //newNode->name = *(findName(completePath));
  for (int i = 0; i < strlen(findName(completePath));i++) {
    newNode->name[i] = findName(completePath)[i];
  }
  newNode->name[strlen(findName(completePath))] = NULL;

  printf("%s\n", "add node got to this point");
}

const char*
findName(const char* completePath) {
  printf("here\n");
  int size = strlen(completePath);
  int indexOfFinalSlash = 0;
  // char* cur =
  // Loop through, saving the location of a slash whenever found.
  for (int i = 0; i < size; i++) {
    //cur[0] = *(completePath + i);
    //printf("%s\n", cur);
    if(streq((const char*) (((void*)completePath) + i), "/")) {
      indexOfFinalSlash = i;
    }
  }
  printf("here\n");
  return (const char*)(((void*)completePath) + indexOfFinalSlash + 1);

}


// States: 0 == off  // 1 == on
void
flip_iNode_bit(int which_iNode, int state) {

  // Assert that we're changing the state to something useful.
  assert(state == 0 || state == 1);

  void* targetPtr = GET_ptr_start_iNode_bitMap() + sizeof(int)*which_iNode;
  //printf("TargetPointer: %d\n", targetPtr);
  //*((int*)(targetPtr)) = 1;
  *((int*)targetPtr) = state;
  //*((int*)targetPtr) = 4;
}

void flip_data_block_bit(int which_block, int state) {
  // Assert that we're changing the state to something useful.
  assert(state == 0 || state == 1);

  void* targetPtr = GET_ptr_start_dataBlock_bitMap() + sizeof(int)*which_block;
  //printf("TargetPointer: %d\n", targetPtr);
  //*((int*)(targetPtr)) = 1;
  *((int*)targetPtr) = state;
  //*((int*)targetPtr) = 4;
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
    return GET_ptr_start_dataBlocks() + 4096 * pnum;
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
        printf("node{refs: %d, mode: %04o, size: %d, xtra: %d, path: %s, name: %s}\n",
               node->refs, node->mode, node->size, node->xtra, node->path, node->name);
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
int GET_NUMBER_OF_INODES(){
  return NUMBER_OF_INODES;
}
int GET_NUMBER_OF_DATABLOCKS() {
  return NUMBER_OF_DATABLOCKS;
}
