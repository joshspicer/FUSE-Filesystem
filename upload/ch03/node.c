//
// Created by Joshua Spicer on 12/3/17.
//

#include "node.h"
#include "datablock.h"

void
add_node(const char *completePath, int mode, int xtra, int which_iNode) {

    void *locationToPlace =
            (sizeof(pnode) * which_iNode) + GET_ptr_start_iNode_Table();

    pnode *newNode = (pnode *) (locationToPlace);
    newNode->mode = mode;
    newNode->xtra = xtra;

    name_node(newNode, completePath);

    // --- Add first open data block array as this inode's chunk of data --

    // TODO support more than 4096 bytes of data per block

    // TODO calculate how many blocks we can allot with our superblock (1MB limit)
    //      so that we don't go over.

    //int MAX_BLOCKS = 10;  //FIXME

    // Keep searching for next available block according to bitmap
    int firstAvailableBlockIdx = -1;
    for (int i = 0; i < GET_NUMBER_OF_DATABLOCKS(); i++) {
        if (*((int *) (GET_ptr_start_iNode_bitMap() + sizeof(int) * i)) == 0) {
            firstAvailableBlockIdx = i;
            break;
        }
    }

    if (firstAvailableBlockIdx != -1) {
        flip_data_block_bit(firstAvailableBlockIdx, 1);
        newNode->blockID = firstAvailableBlockIdx;
        printf("BLOCK %d FOUND FOR INODE\n", firstAvailableBlockIdx);

    } else {
        newNode->blockID = -1;
        printf("%s\n", "NO AVAILABLE BLOCK FOUND FOR THIS INODE.");
    }

    // Size of files within block start as 0 (cuz nothing is there).
    newNode->size = 0;

    // Let the iNode know which node it is in the bitmap.
    // Useful for removing.
    newNode->nodeID = which_iNode;
}

// ---------------------------------------------------------------------------- //

// Find and return file name when given an entire path
// (the name after the last slash)
// HELPER for name_node below
const char *
findName(const char *completePath) {
    int size = strlen(completePath);
    int indexOfFinalSlash = 0;
    // Loop through, saving the location of a slash whenever found.
    for (int i = 0; i < size; i++) {
        if (streq((const char *) (((void *) completePath) + i), "/")) {
            indexOfFinalSlash = i;
        }
    }
    return (const char *) (((void *) completePath) + indexOfFinalSlash + 1);
}

const char *
findPreceedingPath(const char *completePath) {
  int size = strlen(completePath);
  int indexOfFinalSlash = 0;
  // Loop through, saving the location of a slash whenever found.
  for (int i = 0; i < size; i++) {
      if (streq((const char *) (((void *) completePath) + i), "/")) {
          indexOfFinalSlash = i;
      }
  }

  char *preceedingPath;
  memcpy(preceedingPath,completePath,indexOfFinalSlash);

  return preceedingPath;
}


// Used during node initalization to set the name field.
void
name_node(pnode *node, const char *path) {
    for (int i = 0; i < strlen(path); i++) {
        node->path[i] = path[i];
    }
    node->path[strlen(path)] = NULL;
    for (int i = 0; i < strlen(findName(path)); i++) {
        node->name[i] = findName(path)[i];
    }
    node->name[strlen(findName(path))] = NULL;
}

// ---------------------------------------------------------------------------- //


// States: 0 == off  // 1 == on
void
flip_iNode_bit(int which_iNode, int state) {

    // Assert that we're changing the state to something useful.
    assert(state == 0 || state == 1);

    void *targetPtr = GET_ptr_start_iNode_bitMap() + sizeof(int) * which_iNode;
    //printf("TargetPointer: %d\n", targetPtr);
    //*((int*)(targetPtr)) = 1;
    *((int *) targetPtr) = state;
    //*((int*)targetPtr) = 4;
}

// ---------------------------------------------------------------------------- //


int
find_empty_inode_index() {
    for (int i = 0; i < GET_NUMBER_OF_INODES(); i++) {

        // check inode bitmap. If value isn't one, then that inode isn't active.
        if (*((int *) (GET_ptr_start_iNode_bitMap() + sizeof(int) * i)) != 1) {
            return i;
        }
    }
    // If there's no empty slot, return -1
    return -1;
}

// ---------------------------------------------------------------------------- //

void
print_node(pnode *node) {
    if (node) {
        printf("node{mode: %04o, size: %d, xtra: %d, path: %s, name: %s, blockID: %d, InodeID: %d}\n",
                    node->mode, node->size, node->xtra, node->path, node->name, node->blockID, node->nodeID);
    } else {
        printf("node{null}\n");
    }
}
