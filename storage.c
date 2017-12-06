#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <bsd/string.h>

#define FUSE_USE_VERSION 26

#include <fuse.h>

#include "storage.h"
#include "datablock.h"

char*
concat(const char *string1, const char *string2)
{
    char *newStr =
            malloc(strlen(string1) + strlen(string2) + 1);
    strcpy(newStr, string1); strcat(newStr, string2);
    return newStr;
}

// INITIALIZES: the kind of storage to be used by this file system.
void
storage_init(const char *path) {

    superBlock_init(path);
}

// ---------------------------------------------------------------------------- //


// Given a path, searches our storage struture for the given pnode, and returns it.
pnode *
get_file_data(const char *path) {

    //loop through all inodes
    for (int i = 0; i < GET_NUMBER_OF_INODES(); i++) {

        // check inode bitmap. If value isn't one, then that inode isn't active.
        if (*((int *) (GET_ptr_start_iNode_bitMap() + sizeof(int) * i)) != 1) {
            continue;
        }

        // There must be an associated iNode. Calculate address.
        void *currentPtr = ((void *) (GET_ptr_start_iNode_Table() + sizeof(pnode) * i));
        pnode *current = ((pnode *) currentPtr);

        if (streq(path, current->path)) { //If this pnode's path is same.
            printf("File <%s> FOUND in get_file_data\n", path);
            // printf("refs: %d\n", dir->refs);
            // printf("mode: %d\n", dir->mode);
            // printf("size: %d\n", dir->size);
            // printf("xtra: %d\n", dir->xtra);
            // printf("path: %d\n", dir->path);
            // printf("name: %d\n", dir->name);
            // printf("blockID: %d\n", dir->blockID);
            // printf("nodeID: %d\n", dir->nodeID);
            return current;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------- //

// Get the stat's of the node at the given path.
int
get_stat(const char *path, struct stat *st) {

  //printf("----- PATH: %s NLINK: %d  ------ \n",path,st->st_nlink); //REMOVE

    st->st_nlink = 1; //TODO do we need this?!?!?!?!

    pnode *dat = get_file_data(path);
    if (!dat) {
        return -1;
    }

    memset(st, 0, sizeof(struct stat));
    st->st_uid = getuid();
    st->st_gid = getgid();
    st->st_mode = dat->mode;
    st->st_atime = time(NULL);  //TODO time
    st->st_mtime = time(NULL); //TODO time
    st->st_size = dat->size;

    //st_nlink was here was when it was working.
    st->st_nlink = 1;

    //TODO stat with something to do with data blocks.

    return 0;
}

// ---------------------------------------------------------------------------- //

// Get the data stored in the data block of the given Path.
const char *
get_data(const char *path) {

    pnode *node = get_file_data(path);

    if (!node) {
        printf("Node does not exist.\n");
        return 0; // TODO error codes
    }

    int blockID = node->blockID;

    if (blockID == -1) {
        printf("No data block associated with this node.\n");
        return 0; // TODO error codes
    }

    void *blockPtr = data_block_ptr_at_index(blockID);
    if (!blockPtr) {
        return 0; // TODO error codes
    }


    // Mem copy the additonal Blocks (for data >4K)
    for (int i = 0; i < 9; i++) {
      if (node->additionalBlocks[i] != -1) {
        blockPtr = concat(blockPtr, data_block_ptr_at_index(node->additionalBlocks[i]));
      }
    }


    return ((const char *) blockPtr);
}
