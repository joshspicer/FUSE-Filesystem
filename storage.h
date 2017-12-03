#ifndef NUFS_STORAGE_H
#define NUFS_STORAGE_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <bsd/string.h>
#include <stdlib.h>

#include "pages.h"

void storage_init(const char* path);
int         get_stat(const char* path, struct stat* st);
const char* get_data(const char* path);

char* concat(const char *string1, const char *string2);
// Returns the index of the first free iNode, or -1 if no empty inode.
int find_empty_inode_index();
int find_empty_block_index();

pnode* get_file_data(const char* path);



#endif
