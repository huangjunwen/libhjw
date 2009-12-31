#ifndef _DT_HPP_
#define _DT_HPP_

#include "dt.h"

class MyDT {
public:
    MyDT() {
        dt_create(&_dt);
    }
    virtual ~MyDT() {
        dt_destroy(&_dt);
    }
    void begin(edgeHandler handler, void * extra = 0) {
        dt_begin(_dt, handler, extra);
    }
    void next(metric x, metric y, void * v) {
        dt_next(_dt, x, y, v);
    }
    void end() {
        dt_end(_dt);
    }
private:
    myDt _dt;
};

#endif
