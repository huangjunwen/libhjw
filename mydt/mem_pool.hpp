#ifndef _MEM_POOL_HPP_
#define _MEM_POOL_HPP_

#include "mem_pool.h"

// mem_pool's c++ wrapper
template<typename T, unsigned int IncSz = 256>
class MemPool {
public:
    MemPool() { 
        mem_pool_init(&_pool, sizeof(T), IncSz); 
    }
    virtual ~MemPool() { 
        mem_pool_finalize(&_pool); 
    }
    T * getRaw() { 
        return (T *)mem_pool_get(&_pool);
    }
    T * get() {
        return new(mem_pool_get(&pool)) T();
    }
    void release(T * p) { 
        p->~T();
        mem_pool_release(&_pool, p); 
    }
private:
    memPool _pool;
};

#endif
