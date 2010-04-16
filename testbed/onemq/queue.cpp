#include "queue.hpp"

namespace omq {

queue_t::queue_t() {
    uuid_clear(_id);
}

bool queue_t::set_id(const uuid_t id) {
    if (!uuid_is_null(_id))
        return false;
    uuid_copy(_id, id);
    return true;
}

const uuid_t * queue_t::get_id() {
    return &_id;
}

}
