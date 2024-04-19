#include "wfs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

char image_file[256];
size_t num_data_blocks;
size_t num_inodes;

int main(int argc, char *argv[]) {
    int op;
	while ((op = getopt(argc, argv, "d:i:b:")) != -1) {
		switch (op) {

		case 'b':
		num_data_blocks = atoi(optarg);
		break;

		case 'i':
		num_inodes = atoi(optarg);
		break;
	
		case 'd':
		strcpy(image_file, optarg);
		break;

		default:
		return 1;
		}
	}
    printf("Initializing %s with WFS with %lu inodes and %lu data blocks",image_file,num_inodes,num_data_blocks);
	int fd = open(image_file, O_RDWR);
	FILE *fp = fopen(image_file, "r");
	fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    __UINT8_TYPE__ *mem = mmap(0x00, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	fclose(fp);

	struct wfs_sb *sb = (struct wfs_sb *)mem;
	memset(mem, 0, size);
	sb->num_inodes = num_inodes;
	sb->num_data_blocks = num_data_blocks;
	sb->i_bitmap_ptr = 0 + BLOCK_SIZE;
	sb->d_bitmap_ptr = 0 + (BLOCK_SIZE * 2);
	sb->i_blocks_ptr = 0 + (BLOCK_SIZE * 3);
	sb->d_blocks_ptr = 0 + (BLOCK_SIZE * 4);
	struct wfs_inode root_inode = {
		.num = 0, // inode 0
		.mode = 0, // directory
		.size = 2 * sizeof(struct wfs_dentry), // Directory has 2 items
		.nlinks = 2, // Directory references itself twice
		.atim = time(NULL),
		.mtim = time(NULL),
		.ctim = time(NULL),
		.blocks[0] = sb->d_blocks_ptr // Reference to actual data for this directory
	};
	memcpy(mem + sb->i_blocks_ptr, &root_inode, sizeof(struct wfs_inode));
	struct wfs_dentry dot_entry = {.name = ".", .num = 0};
	struct wfs_dentry dotdot_entry = {.name = "..", .num = 0};
	memcpy(mem + sb->d_blocks_ptr, &dot_entry, sizeof(struct wfs_dentry));
	memcpy(mem + sb->d_blocks_ptr + sizeof(struct wfs_dentry), &dotdot_entry, sizeof(struct wfs_dentry));
	memset(mem + sb->i_bitmap_ptr, 0x80, 1);
	memset(mem + sb->d_bitmap_ptr, 0x80, 1);
	munmap(mem, size);
}