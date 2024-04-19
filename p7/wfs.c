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
        return -ENOENT;
    }
}

static int my_getattr(const char *path, struct stat *stbuf) {
    printf("getattr %s\n",path);
    int inodeNum = getInodeFromPath(path);
    if (inodeNum == -ENOENT) {
        return -ENOENT;
    }
    
    struct wfs_inode *inode = (struct wfs_inode *)(mem + sb->i_blocks_ptr + 128 * inodeNum);
    
    stbuf->st_mode = inode->mode;
    stbuf->st_uid = inode->uid;
    stbuf->st_gid = inode->gid;
    stbuf->st_size = inode->size;
    stbuf->st_nlink = inode->nlinks;
    stbuf->st_atime = inode->atim;
    stbuf->st_mtime = inode->mtim;
    stbuf->st_ctime = inode->ctim;
    return 0;
}

static int wfs_mknod(const char* path, mode_t mode, dev_t rdev) {
    printf("mknod\n");
    return 0;
}

static int wfs_mkdir(const char* path, mode_t mode) {
    printf("mkdir\n");
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
