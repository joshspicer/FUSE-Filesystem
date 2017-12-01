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



typedef struct file_data {
    const char* path;
    int         mode;
    const char* data;

} file_data;

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
    printf("TODO: Store file system data in: %s\n", path);

    pages_init(path);

    printf("GOOD: %d\n",GET_OFFSET_start_dataBlocks());


    //int dataRV = open(path,O_CREAT|O_APPEND, S_IRWXU);  //TODO
    // TEST
     //char buffer[10];
     //read(dataRV, buffer,10);
     ///printf("%s\n", buffer);
     //  char newBuffer = "newBuffer";
     // printf("%s\n",strerror(errno));
      //int reval = write(dataRV, &newBuffer, 5);
    //  printf("%s\n",strerror(errno));
     // printf("%s%d\n","RETURN: ",reval);

}

static int
streq(const char* aa, const char* bb)
{
    return strcmp(aa, bb) == 0;
}

// TODO Rewrite get_file_data  and use OUR file storage system
//      instead of file_table[]

static file_data*
get_file_data(const char* path) {

  printf("%s\n","RE-IMPLEMENT GET_FILE_DATA");

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
    file_data* dat = get_file_data(path);
    if (!dat) {
        return -1;
    }

    memset(st, 0, sizeof(struct stat));
    st->st_uid  = getuid();
    st->st_mode = dat->mode;
    if (dat->data) {
        st->st_size = strlen(dat->data);
    }
    else {
        st->st_size = 0;
    }
    return 0;
}

const char*
get_data(const char* path)
{
    file_data* dat = get_file_data(path);
    if (!dat) {
        return 0;
    }

    return dat->data;
}
