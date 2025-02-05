#include "ring_buffer.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>

pthread_mutex_t lock;

int init_ring(struct ring *r) {
    // printf("init ring\n");
    r->p_head = 0;
    r->p_tail = 0;
    r->c_head = 0;
    r->c_tail = 0;
    pthread_mutex_init(&lock, NULL);
    return 0;
}

// client submits new requests to buffer through this
void ring_submit(struct ring *r, struct buffer_descriptor *bd) {
    while (1) {
        pthread_mutex_lock(&lock);
        if ((r->p_head + 1) % RING_SIZE == r->p_tail) {
            pthread_mutex_unlock(&lock);
            continue;
        }
        memcpy(&r->buffer[r->p_head], bd, sizeof(struct buffer_descriptor));
        r->p_head = (r->p_head + 1) % RING_SIZE;
        pthread_mutex_unlock(&lock);
        break;
    }
}

// server requests from the buffer using this
void ring_get(struct ring *r, struct buffer_descriptor *bd) {
    while (1) {
        pthread_mutex_lock(&lock);
        if (r->p_head == r->p_tail) {
            pthread_mutex_unlock(&lock);
            continue;
        }
        memcpy(bd, &r->buffer[r->p_tail], sizeof(struct buffer_descriptor));
        r->p_tail = (r->p_tail + 1) % RING_SIZE;
        pthread_mutex_unlock(&lock);
        break;
    }
}