//
// Created by Joshua Spicer on 12/3/17. .
//

#include "datablock.h"


void flip_data_block_bit(int which_block, int state) {
    // Assert that we're changing the state to something useful.
    assert(state == 0 || state == 1);

    void *targetPtr = GET_ptr_start_dataBlock_bitMap() + sizeof(int) * which_block;
    *((int *) targetPtr) = state;
}

// ---------------------------------------------------------------------------- //

void *
data_block_ptr_at_index(int index) {
    return GET_ptr_start_dataBlocks() + (4096 * index);
}

// ---------------------------------------------------------------------------- //

// Given a block ID, will change ALL inodes pointing to this blockID
// with the given size.
void
correctSizeForLinkedBlocks(int givenBlockID, int size) {

  for (int i = 0; i < GET_NUMBER_OF_INODES();i++) {
    // Node exists!
    if(*((int *) (GET_ptr_start_iNode_bitMap() + sizeof(int) * i)) == 1) {
      pnode* tmp = ((pnode *) (GET_ptr_start_iNode_Table() + sizeof(pnode) * i));
      // If this node is referring to "givenBlockID"
      if (tmp->blockID == givenBlockID) {
        // Set the size to that size.
        tmp->size = size;
      }
    }
  }
}

// ---------------------------------------------------------------------------- //



// TODO: implement find_empty_block_index() and replace my other stuff with it
