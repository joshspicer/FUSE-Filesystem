#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <bsd/string.h>
#include <assert.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "storage.h"

//Josh: Included one of his hints
#include "pages.h"

// typedef struct file_data {
//     const char* path;
//     int         mode;
//     const char* data;
//
// } file_data;

// Josh: This struct is replaced by our data.nufs storage solution.

// static file_data file_table[] = {
//     {"/", 040755, 0},
//     {"/hello.txt", S_IFREG  /*| S_IWUSR*/ /*0100644*/, "hello\n"},
//     {"/josh.txt", S_IFREG, "maddie\n"},
//     {0, 0, 0},
// };

void
storage_init(const char* path)
{
    //printf("TODO: Store file system data in: %s\n", path);

    pages_init(path);

    // TEST
    // printf("1stGetter: %d\n",GET_ptr_start_iNode_bitMap());
    // printf("2stGetter: %d\n",GET_ptr_start_dataBlock_bitMap());
    // printf("3stGetter: %d\n",GET_ptr_start_iNode_Table());
    // printf("4stGetter: %d\n",GET_ptr_start_dataBlocks());

    // int dataRV = open(path,O_CREAT|O_APPEND, S_IRWXU);  //TODO
    // TEST
    // char buffer[10];
    // read(dataRV, buffer,10);
    // printf("%s\n", buffer);
    // char newBuffer = "newBuffer";
    // printf("%s\n",strerror(errno));
    // int reval = write(dataRV, &newBuffer, 5);
    // printf("%s\n",strerror(errno));
    // printf("%s%d\n","RETURN: ",reval);

}

static int
streq(const char* aa, const char* bb)
{
    return strcmp(aa, bb) == 0;
}

// TODO Rewrite get_file_data  and use OUR file storage system
//      instead of file_table[]

static pnode*
get_file_data(const char* path) {

  //loop through all inodes
  for (int i = 0; i < GET_NUMBER_OF_INODES(); i++) {
    // check inode bitmap. If value isn't one, then that inode isn't active.
    if(*((int*)(GET_ptr_start_iNode_bitMap() + sizeof(int)*i)) != 1) {
      continue;
    }

    void* currentPtr = ((void*)(GET_ptr_start_iNode_Table() + sizeof(pnode)*i));
    pnode* current = ((pnode*)currentPtr);

    if (streq(path, current->path) == 0) { //If this pnode's path is same.
      printf("%s\n","File with given path FOUND in get_file_data");
      return current;
    }
  }

    // for (int ii = 0; 1; ++ii) {
    //     file_data row = file_table[ii];
    //
    //     if (file_table[ii].path == 0) {
    //         break;
    //     }
    //
    //     if (streq(path, file_table[ii].path)) {
    //         return &(file_table[ii]);
    //     }
    // }

    return 0;
}

int
get_stat(const char* path, struct stat* st)
{
    pnode* dat = get_file_data(path);
    if (!dat) {
        return -1;
    }

    memset(st, 0, sizeof(struct stat));
    st->st_uid  = getuid();
    st->st_mode = dat->mode;
    st->st_atime = time( NULL );
    st->st_mtime = time( NULL );
    st->st_size = dat->size;

    //TODO stat with something to do with data blocks....

    return 0;
}

const char*
get_data(const char* path)
{
  //TODO actually getting data from the pnodes and sharing that.


    // pnode* dat = get_file_data(path);
    // if (!dat) {
    //     return 0;
    // }

    //return dat->data;
    return "helpme";
}
