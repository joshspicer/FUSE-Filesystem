//
// Created by Joshua Spicer on 12/3/17.
//

#include "datablock.h"


void flip_data_block_bit(int which_block, int state) {
    // Assert that we're changing the state to something useful.
    assert(state == 0 || state == 1);

    void *targetPtr = GET_ptr_start_dataBlock_bitMap() + sizeof(int) * which_block;
    //printf("TargetPointer: %d\n", targetPtr);
    //*((int*)(targetPtr)) = 1;
    *((int *) targetPtr) = state;
    //*((int*)targetPtr) = 4;
}

// ---------------------------------------------------------------------------- //

void *
data_block_ptr_at_index(int index) {
    return GET_ptr_start_dataBlocks() + (4096 * index);
}

// ---------------------------------------------------------------------------- //


// TODO: implement find_empty_block_index() and replace my other stuff with it
