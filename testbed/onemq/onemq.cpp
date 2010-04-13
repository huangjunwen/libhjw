#include "onemq.hpp"

using namespace omq;

queue_t::queue_t(const uuid_t id) {
    uuid_copy(_id, id);
}

const uuid_t * queue_t::get_id() {
    return &_id;
}

#if 0

#include <iostream>

int main() {
    std::cout << sizeof(msg_id_t) << std::endl;
    uuid_t a;
    uuid_parse("d1712bb4-46ec-11df-b24b-000c29bb2336", a);
    msg_id_t mid;
    mid.src = &a;
    mid.seq = 0;
    return 0;
}
#endif
