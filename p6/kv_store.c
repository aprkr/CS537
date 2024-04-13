#include "common.h"
#include "ring_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

int table_size;
int num_threads;
char *optarg;

int put(key_type k, value_type v) {
    printf("server put\n");
    return 0;

}

int get(key_type k) {
    printf("server get\n");
    return 0;

}

int main(int argc, char *argv[]) {
    int op;
	while ((op = getopt(argc, argv, "n:s:v")) != -1) {
		switch (op) {
																	           
		case 'n':
		num_threads = atoi(optarg);
		break;
	
		case 's':
		table_size = atoi(optarg);
		break;

		case 'v':
		break;
		default:
        printf("Wrong usage");
		return 1;
		}
	}
	int fd = open("shmem_file", O_RDWR);
    void *mem = mmap(NULL, sizeof(struct ring), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    struct ring *ring = (struct ring *)mem;

    while (1) {
        struct buffer_descriptor bd;
        ring_get(ring, &bd);
        printf("%d\n",bd.res_off);
        if (bd.req_type == PUT) {
            put(bd.k, bd.v);
        } else {
            get(bd.k);
        }
        // struct buffer_descriptor *idk = ring + bd.res_off;
        struct buffer_descriptor *result = mem + bd.res_off;
        bd.ready = 1;
        memcpy(result, &bd, sizeof(struct buffer_descriptor));
    }
}