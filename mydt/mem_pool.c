// vim:fdm=marker:nu:nowrap
#include <assert.h>
#include <stdlib.h>
#include "mem_pool.h"

/*********************************
 * Mem pool
 *********************************/
static const uint32_t memSegBaseSz = (uint32_t)(((memSeg*)0)->mem);

memSeg * _new_seg(memPool * pool) {
    uint32_t sz = memSegBaseSz + pool->item_sz * pool->inc_sz;
    memSeg * ms = (memSeg *)malloc(sz);
    if (!ms)
        return 0;
    ms->next = 0;
    ms->addr_after_last = (byte_t *)ms + sz;
    return ms;
}

void mem_pool_reset(memPool * pool) {
    pool->free_list = 0;
    pool->next_addr = pool->head_seg.mem;
    pool->curr_seg = &pool->head_seg;
}

boolean_t mem_pool_init(memPool * pool, uint32_t item_sz, uint32_t inc_sz) {
    assert(item_sz >= sizeof(void *));
    assert(inc_sz > 0);
    pool->item_sz = item_sz;
    pool->inc_sz = inc_sz;

    // special init for head_seg
    pool->head_seg.next = 0;
    pool->head_seg.addr_after_last = pool->head_seg.mem;

    mem_pool_reset(pool);
    return 1;
}

void mem_pool_finalize(memPool * pool) {
    memSeg * ms = pool->head_seg.next;
    memSeg * n;
    while (ms) {
        n = ms->next;
        free(ms);
        ms = n;
    }
}

void * mem_pool_get(memPool * pool) {
    void * ret;
    // first check the free list
    if (pool->free_list) {
        ret = pool->free_list;
        pool->free_list = *(void **)pool->free_list;
        return ret;
    }
    // then try to get the next item
    if (pool->next_addr >= pool->curr_seg->addr_after_last) {
        memSeg * curr = pool->curr_seg;
        if (!curr->next) {
            if (!(curr->next = _new_seg(pool)))
                return 0;
        }
        pool->curr_seg = curr->next;
        pool->next_addr = pool->curr_seg->mem;
    }
    ret = pool->next_addr;
    pool->next_addr += pool->item_sz;
    return ret;
}

void mem_pool_release(memPool * pool, void * item) {
    *(void **)item = pool->free_list;
    pool->free_list = item;
}
