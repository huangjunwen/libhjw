#pragma once

#include "dt.h"

template<typename T, typename F>
class MyDT {
public:
    MyDT() {
        dt_create(&_dt);
    }
    virtual ~MyDT() {
        dt_destroy(&_dt);
    }
    void begin(F & cb) {
        _cb = &cb;
        dt_begin(_dt, MyDT<T, F>::_edge_handler, this);
    }
    void next(metric x, metric y, T * v) {
        dt_next(_dt, x, y, v);
    }
    void end() {
        dt_end(_dt);
    }
private:
    static void _edge_handler(void * extra, const node * n1, const node * n2) {
        (*((MyDT<T, F> *)extra)->_cb)((T *)n1->attr, (T *)n2->attr);
    }
    myDt _dt;
    F * _cb;
};
