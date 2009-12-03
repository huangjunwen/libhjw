#pragma once
#include "mem_pool.h"

template<typename T, unsigned int IncSz = 256>
class MemPool {
public:
    MemPool() { 
        mem_pool_init(&_pool, sizeof(T), IncSz); 
    }
    virtual ~MemPool() { 
        mem_pool_finalize(&_pool); 
    }
    T * get() { 
        return (T *)mem_pool_get(&_pool);
    }
    void release(T * p) { 
        mem_pool_release(&_pool, p); 
    }
private:
    memPool _pool;
};
