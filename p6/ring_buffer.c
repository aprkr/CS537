#include "ring_buffer.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

int init_ring(struct ring *r) {
    printf("init ring\n");
    r->p_head = 0;
    r->p_tail = 0;
    r->c_head = 0;
    r->c_tail = 0;
    return 0;
}

// client submits new requests to buffer through this
void ring_submit(struct ring *r, struct buffer_descriptor *bd) {
    printf("ring submit %d\n",bd->k);
    while (1) {
        int p_head = r->p_head;
        int next_index = p_head + 1;
        if (next_index >= RING_SIZE) {
            next_index = 0;
        }

        if (next_index == r->c_tail) {
            return; // full
        }
        if (!__atomic_compare_exchange(&r->p_head,&p_head,&next_index,false,__ATOMIC_RELAXED,__ATOMIC_RELAXED)) {
            continue;
        }
        memcpy(&r->buffer[p_head], bd, sizeof(struct buffer_descriptor));
        while (r->p_tail != p_head) {}
        r->p_tail = r->p_head;
        break;
    }
    return;
}

// server requests from the buffer using this
void ring_get(struct ring *r, struct buffer_descriptor *bd) {
    printf("ring get\n");
    while (1) {
        if (r->c_head == r->p_tail) {
            continue;
        }
        int next_index = r->c_tail + 1;
        if (next_index >= RING_SIZE) {
            next_index = 0;
        }
        memcpy(bd, &r->buffer[r->c_head], sizeof(struct buffer_descriptor));
        r->c_head = next_index;
        r->c_tail = r->c_head;
        break;
    }
    
}