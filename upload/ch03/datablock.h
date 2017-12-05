//
// Created by Joshua Spicer on 12/3/17.
//

#ifndef CHALLENGE03_DATABLOCK_H
#define CHALLENGE03_DATABLOCK_H

#include "superblock.h"

// --- FUNCTIONS ---
void *data_block_ptr_at_index(int index);

int find_empty_block_index();

void flip_data_block_bit(int which_block, int state); // 0 == off, 1 == on

void correctSizeForLinkedBlocks(int blockID, int size);


#endif //CHALLENGE03_DATABLOCK_H
