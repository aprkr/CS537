// #include "wfs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

char image_file[256];
size_t num_data_blocks;
size_t num_inodes;

int main(int argc, char *argv[]) {
    int op;
	while ((op = getopt(argc, argv, "d:i:b:")) != -1) {
		switch (op) {

		case 'b':
		num_data_blocks = atoi(optarg); //
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
}