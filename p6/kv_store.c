#include "common.h"
#include "ring_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>

int table_size;
int num_threads;
char *optarg;
#define MAX_THREADS 128
pthread_t threads[MAX_THREADS];
struct ring *ring;
void *mem;

key_type *key_array;
value_type *value_array;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

int put(key_type k, value_type v) {
    index_t index = hash_function(k, table_size);
    pthread_mutex_lock(&lock2);
    value_array[index] = v;
    key_array[index] = k;
    pthread_mutex_unlock(&lock2);
    return 0;

}

int get(key_type k) {
    index_t index = hash_function(k, table_size);
    if (key_array[index] != 0) {
        return value_array[index];
    } else {
        return 0;
    }

}

void *thread_func(void *arg) {
    // printf("thread started\n");
    while (1) {
        struct buffer_descriptor bd;
        ring_get(ring, &bd);
        if (bd.req_type == PUT) {
            put(bd.k, bd.v);
        } else {
            bd.v = get(bd.k);
        }
        struct buffer_descriptor *result = mem + bd.res_off;
        memcpy(result, &bd, sizeof(struct buffer_descriptor));
        // printf("New completion, %u %u %u\n", bd.req_type, bd.k, bd.v);
        result->ready = 1;
    }
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
        key_array = calloc(table_size, sizeof(key_type));
        value_array = calloc(table_size, sizeof(key_type));
		break;

		case 'v':
		break;
		default:
        printf("Wrong usage");
		return 1;
		}
	}
	int fd = open("shmem_file", O_RDWR);
    mem = mmap(NULL, sizeof(struct ring), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    ring = (struct ring *)mem;
    for (int i = 0; i < num_threads; i++)
		pthread_create(&threads[i], NULL, thread_func, NULL);
    for (int i = 0; i < num_threads; i++)
		if (pthread_join(threads[i], NULL))
			perror("pthread_join");    
}