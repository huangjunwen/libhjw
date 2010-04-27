#ifndef _INTERFACE_HPP_
#define _INTERFACE_HPP_

#include <stdint.h>
#include <uuid/uuid.h>

namespace omq {

#define NULL_SEQ (0)


class i_msg_t;
class i_msg_provider_t;
class i_msg_consumer_t;

typedef struct msg_id_t {
    msg_id_t(): uuid(NULL), seq(NULL_SEQ) {}

    // UUID of the msg source. (queue)
    const uuid_t * uuid;                                 

    // Monotonic increasing number start from 1. 
    // 0 is reserved as NULL_SEQ.
    // Seq maybe not continuous. (filter)
    uint64_t seq;                                       
} msg_id_t;

class i_msg_t {
public:
    msg_id_t id;
};

class i_msg_provider_t {
public:
    // Msg provider is reponsable to provide (queued) msg. 
    // A provider can bind ONLY ONE consumer. 
    //   The consumer's `msg_provided` should be called when some msg is available.
    virtual bool bind_consumer(i_msg_consumer_t *) = 0;
    virtual bool msg_consumed(i_msg_consumer_t *, const msg_id_t *) = 0;
};

class i_msg_consumer_t {
public:
    // Called when a msg is arrived to this device.
    virtual void msg_provided(i_msg_provider_t *, const msg_id_t *) = 0;
};

#if 0
class i_dispatcher_t: public i_consumer_t {
public:
    virtual bool add_worker(i_consumer_t *) = 0;
};
#endif





}

#endif // _INTERFACE_HPP_
