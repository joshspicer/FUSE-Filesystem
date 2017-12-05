
#define _GNU_SOURCE

#include <string.h>

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "superblock.h"

const int NUFS_SIZE = 1024 * 1024; // 1MB


void
superBlock_init(const char *path)
{

    pages_fd = open(path, O_CREAT | O_RDWR, 0644);
    assert(pages_fd != -1);

    int rv = ftruncate(pages_fd, NUFS_SIZE);
    assert(rv == 0);

    pages_base = mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, pages_fd, 0);
    assert(pages_base != MAP_FAILED);

    // --- CREATE SUPERBLOCK ASSIGNING OFFSETS ---
    // Start location of iNode
    SUPER_SIZE = 20;
    NUMBER_OF_INODES = 800;
    NUMBER_OF_DATABLOCKS = 238 / 10;

    //TODO calculate how much space we have for 4k data blocks, and make that many!

    start_iNode_bitMap = SUPER_SIZE;
    start_dataBlock_bitMap = start_iNode_bitMap
                             + NUMBER_OF_INODES * sizeof(int);
    start_iNode_Table = start_dataBlock_bitMap
                        + NUMBER_OF_DATABLOCKS * sizeof(int);
    start_dataBlocks = start_iNode_Table
                       + NUMBER_OF_INODES * sizeof(pnode);

    // Calculate how many Data blocks we have space for.
    printf("Start of DataBlock: %d\n", start_dataBlocks);
    int MaxSizeOfDataSection = NUFS_SIZE - start_dataBlocks;
    printf("NUFS Size - start_dataBlocks: %d\n", MaxSizeOfDataSection);
    

    // Write offset to start of inode bitmap in the superblock
    write_int_offset(0, start_iNode_bitMap);
    // Write offset to start of data block bitmap in the superblock
    write_int_offset(1, start_dataBlock_bitMap);
    // Write offset to start of inode table in the superblock
    write_int_offset(2, start_iNode_Table);
    // Write offset to start of data blocks in the superblock
    write_int_offset(3, start_dataBlocks);

    // Init the root directory.
    add_node("/", 040755, 145, 0);
    flip_iNode_bit(0, 1);
    print_node((pnode *) (GET_ptr_start_iNode_Table() + sizeof(pnode) * 0));

    // add_node("/aaa.txt",S_IFREG | S_IRWXU,55,1);
    // flip_iNode_bit(1,1);
}


// Write the given int to the page_base + offset amount of integers.
void
write_int_offset(int offset, int data) {
    *((int *) (pages_base + sizeof(int) * offset)) = data;
}

// *** GETTERS ***
void *
GET_ptr_start_iNode_bitMap() {
    return start_iNode_bitMap + pages_base;
}

void *
GET_ptr_start_dataBlock_bitMap() {
    return start_dataBlock_bitMap + pages_base;
}

void *
GET_ptr_start_iNode_Table() {
    return start_iNode_Table + pages_base;
}

void *
GET_ptr_start_dataBlocks() {
    return start_dataBlocks + pages_base;
}

int GET_NUMBER_OF_INODES() {
    return NUMBER_OF_INODES;
}

int GET_NUMBER_OF_DATABLOCKS() {
    return NUMBER_OF_DATABLOCKS;
}
