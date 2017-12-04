#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <bsd/string.h>

#define FUSE_USE_VERSION 26

#include <fuse.h>

#include "storage.h"
#include "superblock.h"
#include "datablock.h"
#include "node.h"
#include "storage.h"
#include "slist.h"

// implementation for: man 2 access
// Checks if a file exists.
int
nufs_access(const char *path, int mask) {
    printf("access(%s, %04o)\n", path, mask);
    return 0;
}

// implementation for: man 2 stat
// gets an object's attributes (type, permissions, size, etc)
int
nufs_getattr(const char *path, struct stat *st) {
    printf("getattr(%s)\n", path);

    int rv = get_stat(path, st);
    printf("Path: <%s>. Error code for getattr: %d\n",path,rv); //REMOVE
    //printf("STAT numHardLinks for %s is %d \n", path,st->st_nlink); //REMOVE

    if (rv == -1) {
      //return 0;
      return -ENOENT;
    } else {
        return 0;
    }

// method over.
}

// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi) {
    struct stat st;
    printf("readdir(%s)\n", path);

    get_stat("/", &st); //get_stat(path, &st);

    // filler is a callback that adds one item to the result
    // it will return non-zero when the buffer is full
    int rv = filler(buf, ".", &st, 0);
    assert(rv == 0);

    // Loop through current iNodes and present their data

    for (int i = 0; i < GET_NUMBER_OF_INODES(); i++) {

        //TODO: Once we implement directories, this has to change!!

        // check inode bitmap. If value isn't one, then that inode isn't active.
        if (*((int *) (GET_ptr_start_iNode_bitMap() + sizeof(int) * i)) != 1) {
            continue;
        }
        // There must be an associated iNode. Calculate address.
        void *currentPtr = ((void *) (GET_ptr_start_iNode_Table() + sizeof(pnode) * i));
        pnode *current = ((pnode *) currentPtr);

        if (!(streq(current->path, "/"))) {

            //printf("CUR_PATH: %s, NODE PRECEEDING: %s\n",path, findPreceedingPath(current->path));

            get_stat(current->path, &st);
            filler(buf, current->name, &st, 0);
        }
    }

    // get_stat("/hello.txt", &st);
    // filler(buf, "hello.txt", &st, 0);

    return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev) {

    //open(path, O_CREAT, mode); --- maybe for mode do S_IRWXU | mode
    //link()

    printf("mknod(%s, %04o)\n", path, mode);

    int index = find_empty_inode_index();

    // If there's no free iNodes, return -1
    if (index == -1) {
        printf("%s\n", "ERROR: No free iNode slots.  Cannot mknod");
        return -1;
    }

    flip_iNode_bit(index, 1);
    add_node(path, mode, 0, index);

    print_node((pnode *) (GET_ptr_start_iNode_Table() + sizeof(pnode) * index));

    return 0;  // was -1
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
nufs_mkdir(const char *path, mode_t mode) {
    printf("mkdir(%s) with mode <%d>\n", path, mode);

    int nodeID = find_empty_inode_index();
    if (nodeID == -1) {
      return -1; //TODO error codes?
    }

    flip_iNode_bit(nodeID, 1);
    add_node(path, S_IFDIR|S_IRWXU, 16, nodeID);

    return 0; //was -1
}

int
nufs_unlink(const char *path) {
    printf("unlink(%s)\n", path);

    pnode *node = get_file_data(path);

    if (!node) {
        printf("Cannot remove path because path does not exist.\n");
    }

    flip_iNode_bit(node->nodeID, 0);
    flip_data_block_bit(node->blockID, 0);

    return 0;
}

int
nufs_rmdir(const char *path) {
    printf("rmdir(%s)\n", path);

    // pnode* node = get_file_data(path);
    // print_node(node);
    //
    // if (!node) {
    //   printf("Cannot remove path because path does not exist.\n");
    // }
    //
    // flip_iNode_bit(node->nodeID, 0);
    // flip_data_block_bit(node->blockID, 0);

    return -1;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to) {
    printf("rename(%s => %s)\n", from, to);

    pnode *node = get_file_data(from);

    if (!node) {
        printf("Cannot rename file or directory because it does not exist.\n");
    }

    name_node(node, to);

    return 0;
}

int
nufs_chmod(const char *path, mode_t mode) {
    printf("chmod(%s, %04o)\n", path, mode);
    return -1;
}

int
nufs_truncate(const char *path, off_t size) {

    //  VERY BIG TODO: implement this

    printf("truncate(%s, %ld bytes)\n", path, size);

    //truncate(path, size);

    // printf("%s%s\n","TRUNCATE ERROR:", strerror(errno));
    // return -1;

    return 0;
}

// this is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
int
nufs_open(const char *path, struct fuse_file_info *fi) {
    printf("open(%s)\n", path);
    return 0;
}

// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    printf("read(%s, %ld bytes, @%ld)\n", path, size, offset);
    const char *data = get_data(path);

    int len = strlen(data) + 1; // strlen(data) + 1
    if (size < len) {
        len = size;
    }

    strlcpy(buf, data, len);
    return len;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset,
           struct fuse_file_info *fi) {
    printf("write(%s, %ld bytes, %ld)\n", path, size, offset);

    // Get the node associated with this path.
    pnode *node = get_file_data(path);

    if (!node) {
        printf("%s\n", "YOU CAN'T WRITE TO A FILE THAT DOESN'T EXIST!");
        return -1;  //TODO return error code.
    }

    // We definitely want a positive blockID
    assert(node->blockID != -1);

    // Calculate the memory address of this block's data.
    void *ptr = data_block_ptr_at_index(node->blockID);

    // Write to that memory location (using the given buffer/size/offset)
    //ptr = buf;
    memcpy(ptr, buf, size);
    //int fd = fi->direct_io;

    // TODO Change stat somehow??? (I don't get stat.)

    // Set node's size to size of this file.
    node->size = size;


    return 0;
}

// Update the timestamps on a file or directory.
int
nufs_utimens(const char *path, const struct timespec ts[2]) {
    //   //int rv = storage_set_time(path, ts);
    int rv = -1;
    printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n",
           path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);

    // return rv;
    return 0;
}


// TODO test. Seeing if I can add in additional fuse struct functions
int
josh_nufs_link(const char *target, const char *linkName) {
  printf("%s\n","--------------------LINK CALLED------------------" );

  // Get the target Node.
  pnode* targetNode = get_file_data(target);
  printf("%s\n","TARGET NODE: "); //REMOVE
  print_node(targetNode); //REMOVE

  // Create a new inode with "linkName"
  // int idx = find_empty_inode_index();
  // add_node(linkName, S_IFREG, 640, 5); //TODO change back from 5 to idx
  // flip_iNode_bit(5,1); //same here.

  nufs_mknod(linkName, S_IFREG, 0);
  struct stat st;
  nufs_getattr(linkName,&st);

  // Get that newly created inode.
  pnode* linkedNODE = get_file_data(linkName);

  printf("%s\n","NEW LINK NODE:"); //REMOVE
  print_node(linkedNODE); //REMOVE

  // Set the linkedNode's data block ID to that of target Block.

  printf("BEFORE: targetBlockID: %d, linkedNodeBlockID: %d\n",targetNode->blockID,linkedNODE->blockID);  //REMOVE
  linkedNODE->blockID = targetNode->blockID;
  linkedNODE->size = targetNode->size;

  //  targetNode->blockID = linkedNode->blockID;
  // nufs_getattr(target,&st);
  // nufs_getattr(linkName,&st);

  printf("AFTER: targetBlockID: %d, linkedNodeBlockID: %d\n",targetNode->blockID,linkedNODE->blockID);  //REMOVE


  return 0;  //TODO error checking and returning error codes.
}


void
nufs_init_ops(struct fuse_operations *ops) {
    memset(ops, 0, sizeof(struct fuse_operations));
    ops->access = nufs_access;
    ops->getattr = nufs_getattr;
    ops->readdir = nufs_readdir;
    ops->mknod = nufs_mknod;
    ops->mkdir = nufs_mkdir;
    ops->unlink = nufs_unlink;
    ops->rmdir = nufs_rmdir;
    ops->rename = nufs_rename;
    ops->chmod = nufs_chmod;
    ops->truncate = nufs_truncate;
    ops->open = nufs_open;
    ops->read = nufs_read;
    ops->write = nufs_write;
    ops->utimens = nufs_utimens;

    ops->link = josh_nufs_link;

};

struct fuse_operations nufs_ops;

int
main(int argc, char *argv[]) {
    assert(argc > 2 && argc < 1000);  //1000 was 6
    storage_init(argv[--argc]);
    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}
