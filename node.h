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


// Header file for helpful methods related to creating and modifying NODES (as an atomic unit).

typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    int size; // bytes for file
    int xtra; // more stuff can go here

    // Path of file
    //const char* path;
    char path[32];
    // File rename
    //const char* name;
    char name[32];

    // ID of data block in use.
    // It's simply an index of
    int blockID;
    int nodeID;

} pnode;

// ------- FUNCTIONS -------

void add_node(const char *fullPath, int mode, int xtra, int which_iNode);

void flip_iNode_bit(int which_iNode, int state); // 0 == off, 1 == on

void print_node(pnode *node);

int find_empty_inode_index(); // Returns the index of the first free iNode, or -1 if no empty inode.

const char *findName(const char* path);
const char* findPrecedingPath(const char* path);

void name_node(pnode *node, const char *path);


#endif //CHALLENGE03_NODE_H
