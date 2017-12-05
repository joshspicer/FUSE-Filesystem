//
// Created by Joshua Spicer on 12/3/17.
//

#ifndef CHALLENGE03_NODE_H
#define CHALLENGE03_NODE_H


#include <stdio.h>
#include "util.h"
#include <assert.h>
#include "node.h"
#include "superblock.h"


// Header file for helpful methods related to
// creating and modifying NODES (as an atomic unit).
typedef struct inode {

    int mode;      // permission & type
    int size;      // bytes for file
    int xtra;      // more stuff can go here
    char path[32]; // full file path
    char name[32]; // File name (last part of path).
    int blockID;   // ID of data block this node is referencing to store data.
    int nodeID;    // The ID of THIS node (in the associated bitmap).

    int additionalBlocks[9];  // Additional blocks for data > 4k

} pnode;

// ------- FUNCTIONS ------- //

void add_node(const char *fullPath, int mode, int xtra, int which_iNode);

void flip_iNode_bit(int which_iNode, int state); // 0 == off, 1 == on

void print_node(pnode *node);

void print_additionalBlocks(pnode *node);

int find_empty_inode_index(); // Returns the index of the first free iNode, or -1 if no empty inode.

const char *findName(const char *completePath);

void name_node(pnode *node, const char *path);

const char *findPreceedingPath(const char *completePath);


#endif //CHALLENGE03_NODE_H
