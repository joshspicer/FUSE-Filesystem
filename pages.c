
#define _GNU_SOURCE
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include "pages.h"
#include "slist.h"
#include "util.h"

const int NUFS_SIZE  = 1024 * 1024; // 1MB
const int PAGE_COUNT = 256;


// NOTE: moved this .h file.
// static int   pages_fd   = -1;
// static void* pages_base =  0;

void
pages_init(const char* path)
{

  printf("%s\n","PAGE INIT STARTING.");

    pages_fd = open(path, O_CREAT | O_RDWR, 0644);
    assert(pages_fd != -1);

    int rv = ftruncate(pages_fd, NUFS_SIZE);
    assert(rv == 0);

    pages_base = mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, pages_fd, 0);
    assert(pages_base != MAP_FAILED);

    printf("IN PAGES FD:%d\n", pages_fd);

    char letterJ = 'J';
    write(pages_fd, &letterJ, 1);

}

void
pages_free()
{
    int rv = munmap(pages_base, NUFS_SIZE);
    assert(rv == 0);
}

void*
pages_get_page(int pnum)
{
    return pages_base + 4096 * pnum;
}

pnode*
pages_get_node(int node_id)  //NOTE: changed node_id to pnum
{
    pnode* idx = (pnode*) pages_get_page(0);
    return &(idx[pnum]);
}

int
pages_find_empty()
{
    int pnum = -1;
    for (int ii = 2; ii < PAGE_COUNT; ++ii) {
        if (0) { // if page is empty
            pnum = ii;
            break;
        }
    }
    return pnum;
}

void
print_node(pnode* node)
{
    if (node) {
        printf("node{refs: %d, mode: %04o, size: %d, xtra: %d}\n",
               node->refs, node->mode, node->size, node->xtra);
    }
    else {
        printf("node{null}\n");
    }
}
