// vim:fdm=marker:nu:nowrap

#ifndef _MEM_POOL_H_
#define _MEM_POOL_H_

#include "types.h"

typedef struct memSeg {
    struct memSeg * next;
    byte * addr_after_last;
    byte mem[1];
} memSeg;

typedef struct {
    void * free_list;               // free link list
    byte * next_addr;               // if freelist is null, then point to the next avail addr ( maybe ==addr_after_last )
    memSeg * curr_seg;              // current memSeg
    uint32_t item_sz;               // the size of each item in pool ( must item_sz >= sizeof(void*) )
    uint32_t inc_sz;                // how many item in a memSeg
    memSeg head_seg;                // head of link list of memSeg
} memPool;

boolean mem_pool_init(memPool * pool, uint32_t item_sz, uint32_t inc_sz);
void mem_pool_reset(memPool * pool);
void mem_pool_finalize(memPool * pool);
void * mem_pool_get(memPool * pool);
void mem_pool_release(memPool * pool, void * item);

#endif
