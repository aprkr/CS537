#include "wfs.h"
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

__UINT8_TYPE__ *mem;
struct wfs_sb *sb;
#define BITSPERINT (sizeof(unsigned int) * 8)

static int getInodeFromPath(const char *p) {
    char path[128];
    strcpy(path, p);
    char **names = (char **)malloc(128 * sizeof(char*));
    int numslashes = 0;
    char *input = strtok(path, "/");
    while(input != NULL) {
        names[numslashes] = input;
        numslashes++;
        input = strtok(NULL, " ");
    }
    names = (char **)realloc(names, numslashes * sizeof(char*));
    if (numslashes == 0) {
        return 0;
    } else {
        int curInodeNum = 0;
        for (int i = 0; i < numslashes; i++) {
            struct wfs_inode *curInode = (struct wfs_inode *)(mem + sb->i_blocks_ptr + 128 * curInodeNum);
            int numEntries = (curInode->size / sizeof(struct wfs_dentry));
            for (int j = 0; j < numEntries; j++) {
                struct wfs_dentry *curEntry = (struct wfs_dentry*)(mem + curInode->blocks[0] + j * sizeof(struct wfs_dentry));
                if (strcmp(curEntry->name, names[i]) == 0) {
                    curInodeNum = curEntry->num;
                    break;
                }
            }
            if (curInode->num == curInodeNum) {
                return -ENOENT;
            }
        }
        return curInodeNum;
    }
}

static int parentInodeFromPath(const char *p, char *child) {
    char path[128];
    strcpy(path, p);
    char *parent = strrchr(path, '/');
    strcpy(child, parent + 1);
    if (parent == path) {
        return 0;
    }
    parent[0] = 0;
    return getInodeFromPath(path);
}

static int allocateInode() {
    unsigned int *ptr = (unsigned int*)(mem + sb->i_bitmap_ptr);
    unsigned int *maxptr = (unsigned int*)(mem + sb->d_bitmap_ptr);
    int num_ints = 0;
    while (ptr < maxptr) {
        if (*ptr == UINTMAX_MAX) {
            ptr++;
            num_ints++;
            continue;
        }
        int index = __builtin_ffs(~*ptr) - 1; // ffs finds first set bit, we invert the current ptr to get index of first 0
        *ptr |= 1 << index;
        int inode = (num_ints * BITSPERINT) + index;
        printf("Allocating inode number %u\n",inode);
        return inode;
    }
    return 0;
}

static int allocateDataBlocks() {
    unsigned int *ptr = (unsigned int*)(mem + sb->d_bitmap_ptr);
    unsigned int *maxptr = (unsigned int*)(mem + sb->i_blocks_ptr);
    int num_ints = 0;
    while (ptr < maxptr) {
        if (*ptr == UINTMAX_MAX) {
            ptr++;
            num_ints++;
            continue;
        }
        int index = __builtin_ffs(~*ptr) - 1; // ffs finds first set bit, we invert the current ptr to get index of first 0
        *ptr |= 1 << index;
        int blockNum = (num_ints * BITSPERINT) + index;
        printf("Allocating data block number %u\n",blockNum);
        return blockNum;
    }
    return 0;
}

static int my_getattr(const char *path, struct stat *stbuf) {
    printf("getattr %s\n",path);
    int inodeNum = getInodeFromPath(path);
    if (inodeNum == -ENOENT) {
        printf("Path not found\n");
        return -ENOENT;
    }
    
    struct wfs_inode *inode = (struct wfs_inode *)(mem + sb->i_blocks_ptr + 128 * inodeNum);
    printf("Got inode %u with size %lu\n",inodeNum,inode->size); fflush(stdout);
    
    stbuf->st_mode = inode->mode;
    stbuf->st_uid = inode->uid;
    stbuf->st_gid = inode->gid;
    stbuf->st_size = inode->size;
    stbuf->st_atime = inode->atim;
    stbuf->st_mtime = inode->mtim;
    // The ones below aren't required according to the instructions
    // stbuf->st_ctime = inode->ctim;
    // stbuf->st_nlink = inode->nlinks;
    return 0;
}

static struct wfs_inode *createFile(const char *path, mode_t mode) {
    if (getInodeFromPath(path) != -ENOENT) {
        return NULL;
    }
    char child[128];
    int parentInodeNum = parentInodeFromPath(path, child);
    struct wfs_inode *parentInode = (struct wfs_inode *)(mem + sb->i_blocks_ptr + 128 * parentInodeNum);
    int newInodeNum = allocateInode();
    int newDataBlock = allocateDataBlocks();

    struct wfs_inode *newInode = (struct wfs_inode *)(mem + sb->i_blocks_ptr + 128 * newInodeNum);
    newInode->blocks[0] = sb->d_blocks_ptr + newDataBlock * BLOCK_SIZE;
    newInode->mode = mode;
    newInode->size = 0;
    newInode->uid = getuid();
    newInode->gid = getgid();
    newInode->atim = time(NULL);
    newInode->mtim = time(NULL);
    newInode->ctim = time(NULL);
    newInode->num = newInodeNum;

    struct wfs_dentry *newEntry;
    int i;
    for (i = 2; i < 16; i++) {
        newEntry = (struct wfs_dentry *)(mem + parentInode->blocks[0] + sizeof(struct wfs_dentry) * i);
        if (newEntry->name[0] == 0) {
            break;
        }
    }

    strcpy(newEntry->name, child);
    newEntry->num = newInodeNum;

    parentInode->size += sizeof(struct wfs_dentry);
    return newInode;
}

static int wfs_mknod(const char* path, mode_t mode, dev_t rdev) {
    printf("mknod\n");
    struct wfs_inode *new = createFile(path, mode);
    if (new == NULL) {
        return -EEXIST;
    }
    return 0;
}

static int wfs_mkdir(const char* path, mode_t mode) {
    printf("mkdir\n");
    struct wfs_inode *new = createFile(path, mode);
    if (new == NULL) {
        return -EEXIST;
    }
    struct wfs_dentry dot_entry = {.name = ".", .num = new->num};
	struct wfs_dentry dotdot_entry = {.name = "..", .num = 0}; // TODO
	memcpy(mem + new->blocks[0], &dot_entry, sizeof(struct wfs_dentry));
	memcpy(mem + new->blocks[0] + sizeof(struct wfs_dentry), &dotdot_entry, sizeof(struct wfs_dentry));
    new->size = 2 * sizeof(struct wfs_dentry);
    new->mode |= S_IFDIR;
    return 0;
}

static int wfs_unlink(const char* path) {
    printf("unlink\n");
    return 0;
}

static int wfs_rmdir(const char* path) {
    printf("rmdir\n");
    return 0;
}

static int wfs_read(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    printf("read\n");

    return 0;
}

static int wfs_write(const char* path, const char *buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    printf("write\n");
    return 0;
}

static int wfs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
    printf("readdir\n");
    int inodeNum = getInodeFromPath(path);
    struct wfs_inode *inode = (struct wfs_inode *)(mem + sb->i_blocks_ptr + 128 * inodeNum);
    off_t offs = inode->blocks[0];
    struct wfs_dentry *curEntry;
    int num_entries = inode->size / sizeof(struct wfs_dentry);
    for (int i = 0; i < num_entries; i++) {
        curEntry = (struct wfs_dentry *)(mem + offs + i * sizeof(struct wfs_dentry));
        filler(buf, curEntry->name, NULL, 0);
    }
    return 0;
}

static struct fuse_operations ops = {
    .getattr = my_getattr,
    .mknod   = wfs_mknod,
    .mkdir   = wfs_mkdir,
    .unlink  = wfs_unlink,
    .rmdir   = wfs_rmdir,
    .read    = wfs_read,
    .write   = wfs_write,
    .readdir = wfs_readdir,
};

int main(int argc, char *argv[]) {
    int fd = open(argv[1], O_RDWR);
	FILE *fp = fopen(argv[1], "r");
	fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	fclose(fp);
    sb = (struct wfs_sb *)mem;
    return fuse_main(argc - 1, &argv[1], &ops, NULL);
}
