
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

    pages_fd = open(path, O_CREAT | O_RDWR, 0644);
    assert(pages_fd != -1);

    int rv = ftruncate(pages_fd, NUFS_SIZE);
    assert(rv == 0);

    pages_base = mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, pages_fd, 0);
    assert(pages_base != MAP_FAILED);

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

    // Init the root directory.
    add_node("/", 040755,145,0);
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

int
find_empty_inode_index() {
  for (int i = 0; i < GET_NUMBER_OF_INODES(); i++) {

    // check inode bitmap. If value isn't one, then that inode isn't active.
    if(*((int*)(GET_ptr_start_iNode_bitMap() + sizeof(int)*i)) != 1) {
      return i;
    }
  }
  // If there's no empty slot, return -1
  return -1;
}

// int
// find_empty_block_index() {
//   for (int i = 0; i < GET_NUMBER_OF_DATABLOCKS(); i++) {
//
//     // check inode bitmap. If value isn't one, then that inode isn't active.
//     if(*((int*)(GET_ptr_start_dataBlock_bitMap() + sizeof(int)*i)) != 1) {
//       return i;
//     }
//   }
//   // If there's no empty slot, return -1
//   return -1;
// }

void
write_int_offset(int offset, int data) {
    *((int*)(pages_base + sizeof(int)*offset)) = data;
}


// void
// write_char_offset(int offset, char data) {
//     *((char*)pages_base + sizeof(char)*offset) = data;
// }

void
add_node(const char* completePath, int mode, int xtra, int which_iNode) {

  void* locationToPlace =
            (sizeof(pnode)*which_iNode) + GET_ptr_start_iNode_Table();

  pnode* newNode = (pnode*)(locationToPlace);
  newNode->mode = mode;
  newNode->xtra = xtra;

  for (int i = 0; i < strlen(completePath); i++) {
    newNode->path[i] = completePath[i];
  }
  newNode->path[strlen(completePath)] = NULL;
  for (int i = 0; i < strlen(findName(completePath));i++) {
    newNode->name[i] = findName(completePath)[i];
  }
  newNode->name[strlen(findName(completePath))] = NULL;


  // --- Add first open data block array as this inode's chunk of data --

  // TODO support more than 4096 bytes of data per block

  // TODO calculate how many blocks we can allot with our superblock (1MB limit)
  //      so that we don't go over.
  int MAX_BLOCKS = 10;  //FIXME

  // Keep searching for next available block according to bitmap
  int firstAvailableBlockIdx = -1;
  for (int i = 0; i < MAX_BLOCKS; i++) {
    if (*((int*)(GET_ptr_start_iNode_bitMap() + sizeof(int)*i)) == 0) {
      firstAvailableBlockIdx = i;
      break;
    }
  }

  if (firstAvailableBlockIdx != -1) {
    flip_data_block_bit(firstAvailableBlockIdx, 1);
    newNode->blockID = firstAvailableBlockIdx;
    printf("BLOCK %d FOUND FOR INODE\n",firstAvailableBlockIdx);

  } else {
    printf("%s\n","NO AVAILABLE BLOCK FOUND FOR THIS INODE.");
  }

  // Size of files within block start as 0 (cuz nothing is there).
  newNode->size = 0;
}

const char*
findName(const char* completePath) {
  int size = strlen(completePath);
  int indexOfFinalSlash = 0;
  // Loop through, saving the location of a slash whenever found.
  for (int i = 0; i < size; i++) {
    if(streq((const char*) (((void*)completePath) + i), "/")) {
      indexOfFinalSlash = i;
    }
  }
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
data_block_ptr_at_index(int index)
{
    return GET_ptr_start_dataBlocks() + 4096 * index;
}

// pnode*
// pages_get_node(int nodeNum)  //NOTE: changed node_id to nodeNum
// {
//     pnode* idx = (pnode*) pages_get_page(0);
//     return &(idx[nodeNum]);
// }

// int
// pages_find_empty()
// {
//     int pnum = -1;
//     for (int ii = 2; ii < PAGE_COUNT; ++ii) {
//         if (0) { // if page is empty
//             pnum = ii;
//             break;
//         }
//     }
//     return pnum;
// }

void
print_node(pnode* node)
{
    if (node) {
        printf("node{refs: %d, mode: %04o, size: %d, xtra: %d, path: %s, name: %s, blockID: %d}\n",
               node->refs, node->mode, node->size, node->xtra, node->path, node->name, node->blockID);
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
