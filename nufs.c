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
    // printf("UID: %s\n", st->st_uid);
    // printf("GID: %s\n",st->st_gid);
    // printf("MODE: %s\n",st->st_mode);
    // printf("ATIME: %s\n",st->st_atime);  //TODO time
    // printf("MTIME: %s\n",st->st_mtime); //TODO time
    // printf("SIZE: %s\n",st->st_size);
    // printf("NLINK: %s\n",st->st_nlink); //All the work is done in get_stat.
    // printf("RV: %s\n", rv);
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

    // get_stat("/", &st); //get_stat(path, &st);

    get_stat(path, &st);
    // filler is a callback that adds one item to the result
    // it will return non-zero when the buffer is full
    int rv = filler(buf, ".", &st, 0);
    assert(rv == 0);

    // Not sure if I need this?
    if (!streq(path, "/")) {
      get_stat(findPrecedingPath(path), &st);
      filler(buf, "..", &st, 0);
    }

    pnode* node = get_file_data(path);
    printf("/ size: %d\n", node->size);
    int* node_data = ((int*)get_data(path));
    int nodeID = -1;
    printf("LOOPING THROUGH DIRECTORY ENTIRES.\n");
    int count = 0;
    int i = 0;
    while (count < node->size) {
      nodeID = node_data[i];
      printf("NodeID: %d\n", nodeID);
      // File has been removed
      if (nodeID != -1) {
        pnode* dirent = (pnode*) (GET_ptr_start_iNode_Table() + nodeID * sizeof(pnode));
        printf("Entry path: %s\n", dirent->path);
        get_stat(dirent->path, &st);
        filler(buf, dirent->name, &st, 0);
        count++;
      }
      i++;
    }



    // TODO: Loop through current iNodes and present their data

    // for (int i = 0; i < GET_NUMBER_OF_INODES(); i++) {
    //
    //     //TODO: Once we implement directories, this has to change!!
    //
    //     // check inode bitmap. If value isn't one, then that inode isn't active.
    //     if (*((int *) (GET_ptr_start_iNode_bitMap() + sizeof(int) * i)) != 1) {
    //         continue;
    //     }
    //     // There must be an associated iNode. Calculate address.
    //     void *currentPtr = ((void *) (GET_ptr_start_iNode_Table() + sizeof(pnode) * i));
    //     pnode *current = ((pnode *) currentPtr);
    //
    //     if (!(streq(current->path, "/"))) {
    //         get_stat(current->path, &st);
    //         filler(buf, current->name, &st, 0);
    //     }
    // }

    // get_stat("/hello.txt", &st);
    // filler(buf, "hello.txt", &st, 0);
    print_all();

    return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev) {

    printf("mknod(%s, %04o)\n", path, mode);

    int index = find_empty_inode_index();

    // If there's no free iNodes, return -1
    if (index == -1) {
        printf("%s\n", "ERROR: No free iNode slots.  Cannot mknod");
        return -1;
    }

    flip_iNode_bit(index, 1);
    add_node(path, mode, 0, index);
    pnode* newNode = get_file_data(path);

    const char* prec = findPrecedingPath(path);
    printf("Prec Path: %s\n", prec);

    pnode* dir = get_file_data(prec);
    print_node(dir);

    int* nodeIDs = ((int*)get_data(prec));
    nodeIDs[dir->size] = newNode->nodeID;
    dir->size += 1;

    print_node(newNode);

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
    add_node(path, S_IFDIR|S_IRWXU, 0, nodeID);

    pnode* newNode = get_file_data(path);

    const char* prec = findPrecedingPath(path);
    printf("Prec Path: %s\n", prec);

    pnode* dir = get_file_data(prec);
    print_node(dir);

    int* nodeIDs = ((int*)get_data(prec));
    nodeIDs[dir->size] = newNode->nodeID;
    dir->size += 1;

    print_node(newNode);

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

    const char* dirPath = findPrecedingPath(path);
    pnode* dir = get_file_data(dirPath);
    remove_from_dir(dir, node->nodeID);

    return 0;
}

int
nufs_rmdir(const char *path) {
    printf("rmdir(%s)\n", path);

    pnode* node = get_file_data(path);
    print_node(node);

    if (!node) {
      printf("Cannot remove path because path does not exist.\n");
    }

    flip_iNode_bit(node->nodeID, 0);
    flip_data_block_bit(node->blockID, 0);

    const char* dirPath = findPrecedingPath(path);
    pnode* dir = get_file_data(dirPath);
    remove_from_dir(dir, node->nodeID);

    return 0;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to) {
    printf("rename(%s => %s)\n", from, to);

    pnode *node = get_file_data(from);

    if (!node) {
        printf("Cannot rename file or directory because it does not exist.\n");
        return -1;
    }

    const char* fromDir = findPrecedingPath(from);
    const char* toDir = findPrecedingPath(to);
    if (!streq(fromDir, toDir)) {
      // Handles case where we are changing directories
      remove_from_dir(get_file_data(fromDir), node->nodeID);

      pnode* dir = get_file_data(toDir);

      int* nodeIDs = ((int*)get_data(toDir));
      nodeIDs[dir->size] = node->nodeID;
      dir->size += 1;
    }

    name_node(node, to);
    printf("Node name: %s\n", node->name);

    return 0;
}

int
nufs_chmod(const char *path, mode_t mode) {
    printf("chmod(%s, %04o)\n", path, mode);

    pnode *node = get_file_data(path);
    node->mode = mode;
    return 0;
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

    const char *data = get_data(path); //  <----- Now supports >4k

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

    printf("write(%s, %ld bytes, offset %ld)\n", path, size, offset);

    // Get the node associated with this path.
    pnode *node = get_file_data(path);

    if (!node) {
        printf("%s\n", "Can't write. File doesn't exist.");
        return -1;  //TODO return error code.
    }

    // We definitely want a positive blockID
    assert(node->blockID != -1);

    // Calculate the memory address of this block's data.
    void *ptr = data_block_ptr_at_index(node->blockID);

    // Write to that memory location (using the given buffer/size/offset)

    int singleBlockSafeSize = size;
    if (singleBlockSafeSize > 4096) {
      singleBlockSafeSize = 4096;
    }

    memcpy(ptr, buf, singleBlockSafeSize); //TODO add offset.


    // TODO ::: if you want to memcpy more than one page,
    //          recalculate ptr after copying 2048 bytes and then
    //          memcpy again!!!!!!!!!!
    /// TODO :: size/4096 to find out which additionalBlock you need to fill!!!


    // Set node's size to size of this file.
    node->size = size;

    // Fix all the references files inodes!
    correctSizeForLinkedBlocks(node->blockID, size);


    // ---------- Beneath this line supports writing file ----------------- //

    int sizeRemaining = node->size - 4096;
    int looped = 1;

    while (sizeRemaining > 0) {
      printf("Inside over 4K writing block for <%s> ", node->path); //REMOVE
      printf("Size Remaining: %d\n", sizeRemaining); //REMOVE

      void *additionalPtr = data_block_ptr_at_index(node->additionalBlocks[looped]);

      // Mem copy the offset data in 4k increments.
      int sizeToCopy = 4096;
      if (sizeRemaining < 4096) {
        sizeToCopy = sizeRemaining;
      }

      memcpy(additionalPtr, buf + (4096*looped),sizeToCopy);

      sizeRemaining -= 4096;
      looped += 1;

      // TODO
      printf("TODO: %s\n", "implement correctSizeForLinkedBlocks()");
    }

    return 0;
}

// Update the timestamps on a file or directory.
int
nufs_utimens(const char *path, const struct timespec ts[2]) {

    printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n",
           path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec);

    pnode *node = get_file_data(path);

    if (!node) {
        printf("%s\n", "Can't write. File doesn't exist.");
        return -1;  //TODO return error code.
    }


    node->timeStamp = ts->tv_sec;

    return 0;
}


// TODO test. Seeing if I can add in additional fuse struct functions
int
nufs_link(const char *target, const char *linkName) {
  printf("Linking target: <%s> to new file <%s>. \n",target, linkName);

  // Get the target Node.
  pnode* targetNode = get_file_data(target);

  nufs_mknod(linkName, S_IFREG, 0);
  struct stat st;
  nufs_getattr(linkName,&st);

  // Get that newly created inode.
  pnode* linkedNODE = get_file_data(linkName);

  // Set the linkedNode's data block ID to that of target Block.
  linkedNODE->blockID = targetNode->blockID;
  linkedNODE->size = targetNode->size;

  return 0; // ln error checks for us.
}

void print_all() {
  printf("PRINTING ALL\n");
  for (int i = 0; i < GET_NUMBER_OF_INODES(); i++) {

      //TODO: Once we implement directories, this has to change!!

      // check inode bitmap. If value isn't one, then that inode isn't active.
      if (*((int *) (GET_ptr_start_iNode_bitMap() + sizeof(int) * i)) != 1) {
          continue;
      }
      // There must be an associated iNode. Calculate address.
      void *currentPtr = ((void *) (GET_ptr_start_iNode_Table() + sizeof(pnode) * i));
      pnode *current = ((pnode *) currentPtr);
      print_node(current);

      // if (!(streq(current->path, "/"))) {
      //     get_stat(current->path, &st);
      //     filler(buf, current->name, &st, 0);
      // }
  }
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

    ops->link = nufs_link;

};

struct fuse_operations nufs_ops;

int
main(int argc, char *argv[]) {
    assert(argc > 2 && argc < 6);
    storage_init(argv[--argc]);
    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}
