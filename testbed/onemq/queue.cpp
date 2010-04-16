#include "queue.hpp"

namespace omq {

queue_t::queue_t() {
    uuid_clear(id);
}

bool queue_t::set_id(const uuid_t id_) {
    if (!uuid_is_null(id))
        return false;
    uuid_copy(id, id_);
    return true;
}

const uuid_t * queue_t::get_id() {
    return &id;
}

}
