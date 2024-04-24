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
	if ((num_data_blocks % 32) != 0) { // round up to nearest 32 data blocks
		num_data_blocks = num_data_blocks + 32 - (num_data_blocks % 32);
	}
    printf("Initializing %s with WFS with %lu inodes and %lu data blocks\n",image_file,num_inodes,num_data_blocks);
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
	int i_bitmap_numblocks = (((num_inodes + 7) / 8 + BLOCK_SIZE - 1) / BLOCK_SIZE); // each block can hold bitmap for (8 * 512) 4096 items
	int d_bitmap_numblocks = (((num_data_blocks + 7) / 8 + BLOCK_SIZE - 1) / BLOCK_SIZE);
	int i_blocks_numblocks = ((num_inodes * 128 + BLOCK_SIZE - 1) / BLOCK_SIZE); // 4 inodes per block
	sb->i_bitmap_ptr = BLOCK_SIZE;
	sb->d_bitmap_ptr = sb->i_bitmap_ptr + BLOCK_SIZE * i_bitmap_numblocks;
	sb->i_blocks_ptr = sb->d_bitmap_ptr + BLOCK_SIZE * d_bitmap_numblocks;
	sb->d_blocks_ptr = sb->i_blocks_ptr + BLOCK_SIZE * i_blocks_numblocks;
	int required_blocks = 1 + i_bitmap_numblocks + d_bitmap_numblocks + i_blocks_numblocks + num_data_blocks;
	if ((required_blocks * BLOCK_SIZE) > size) {
		printf("Image file not big enough to accomodate specified filesystem, requires %u bytes\n", required_blocks * BLOCK_SIZE);
		return 1;
	}
	struct wfs_inode root_inode = {
		.num = 0, // inode 0
		.mode = S_IFDIR | 0755, // directory
		.size = 0, // Directory has 2 items
		.nlinks = 2, // Directory references itself twice
		.atim = time(NULL),
		.mtim = time(NULL),
		.ctim = time(NULL),
		.blocks[0] = sb->d_blocks_ptr // Reference to actual data for this directory
	};
	memcpy(mem + sb->i_blocks_ptr, &root_inode, sizeof(struct wfs_inode));
	// struct wfs_dentry dot_entry = {.name = ".", .num = 0};
	// struct wfs_dentry dotdot_entry = {.name = "..", .num = 0};
	// memcpy(mem + sb->d_blocks_ptr, &dot_entry, sizeof(struct wfs_dentry));
	// memcpy(mem + sb->d_blocks_ptr + sizeof(struct wfs_dentry), &dotdot_entry, sizeof(struct wfs_dentry));
	memset(mem + sb->i_bitmap_ptr, 0x1, 1);
	memset(mem + sb->d_bitmap_ptr, 0x1, 1);
	munmap(mem, size);
}