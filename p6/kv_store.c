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

typedef struct {
    key_type key;
    value_type value;
} hash_entry;
hash_entry *hash_table;
int ht_cur_length = 0;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

int put(key_type k, value_type v) {
    index_t index = hash_function(k, table_size);
    pthread_mutex_lock(&lock2);
    if (ht_cur_length >= (table_size / 2)) {
        hash_entry *new_hash_table = calloc(table_size * 2, sizeof(hash_entry));
        for(int i = 0; i < table_size; i++) {
            key_type cur_key = hash_table[i].key;
            if (cur_key != 0) {
                index_t new_index = hash_function(cur_key, table_size * 2);
                while(new_hash_table[new_index].key != 0) {
                    new_index++;
                    if (new_index >= (table_size * 2)) {
                        new_index = 0;
                    }
                }
                new_hash_table[new_index].key = cur_key;
                new_hash_table[new_index].value = hash_table[i].value;
            }
        }
        free(hash_table);
        hash_table = new_hash_table;
        table_size = table_size * 2;
    }
    while(hash_table[index].key != 0) {
        if (hash_table[index].key == k) {
            break;
        }
        index++;
        if (index >= table_size) {
            index = 0;
        }
    }
    hash_table[index].value = v;
    hash_table[index].key = k;
    ht_cur_length++;
    pthread_mutex_unlock(&lock2);
    return 0;

}

int get(key_type k) {
    index_t index = hash_function(k, table_size);
    pthread_mutex_lock(&lock2);
    while(hash_table[index].key != 0) {
        key_type key = hash_table[index].key;
        if (key == k) {
            value_type value = hash_table[index].value;
            pthread_mutex_unlock(&lock2);
            return value;
        }
        index++;
        if (index >= table_size) {
            index = 0;
        }
    }
    pthread_mutex_unlock(&lock2);
    return 0;
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
        hash_table = calloc(table_size, sizeof(hash_entry));
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