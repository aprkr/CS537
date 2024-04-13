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
    int next_index = r->p_head + 1;
    // struct buffer_descriptor next = r->buffer[r->p_head + 1];
    if (next_index >= RING_SIZE) {
        next_index = 0;
    }

    if (next_index == r->c_tail) {
        return; // full
    }
    memcpy(&r->buffer[r->p_head], bd, sizeof(struct buffer_descriptor));
    r->p_head = next_index;
    r->p_tail = r->p_head;
    printf("ring submit done\n");
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