// vim:fdm=marker:nu:nowrap:encoding=utf-8

#ifndef _MEM_POOL_H
#define _MEM_POOL_H

#include "types.h"

typedef struct memSeg {
    struct memSeg * next;
    byte * addr_after_last;
    byte mem[0];
} memSeg;

typedef struct {
    void * free_list;               // 归还的空闲链表
    byte * next_addr;               // 若空闲链表为空，则这个指向的是下一个可分配的地址 ( 有可能==addr_after_last )
    memSeg * head_seg;              // memSeg 是一个链表，链表头
    memSeg * curr_seg;              // 当前memSeg
    uint32 item_sz;                 // 顾名思义 ( 必须满足 item_sz >= sizeof(void*) )
    uint32 inc_sz;                  // 每一个seg中包含的item个数
} memPool;

boolean mem_pool_init(memPool * pool, uint32 item_sz, uint32 inc_sz);
void mem_pool_reset(memPool * pool);
void mem_pool_finalize(memPool * pool);
void * mem_pool_get(memPool * pool);
void mem_pool_release(memPool * pool, void * item);

#endif
