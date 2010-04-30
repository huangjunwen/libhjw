#ifndef _INTERFACE_HPP_
#define _INTERFACE_HPP_

#include <stdint.h>
#include <uuid/uuid.h>

namespace omq {


/*******************************************************
 *
 * Messages are tagged. (ordered)
 *
 *******************************************************/
#define NULL_SEQ (0)

typedef struct {
    msg_id_t(): uuid(NULL), seq(NULL_SEQ) {}

    // UUID of the msg source. (queue)
    const uuid_t * uuid;                                 

    // Monotonic increasing number start from 1. 
    // 0 is reserved as NULL_SEQ.
    // Seq maybe not continuous. (filter)
    uint64_t seq;                                       
} msg_id_t;

typedef struct {
    const unsigned char * content;
    size_t len;
    msg_id_t id;
} msg_t;

/*******************************************************
 *
 * Msg provider is responsable to provide (queued) msg. 
 * Msg consumer is responsable to handle msg. 
 *
 *******************************************************/
class i_msg_provider_t;
class i_msg_consumer_t;

class i_msg_provider_t {
public:
    // A provider can bind ONLY ONE consumer. 
    // Unbind the consumer if the pointer passed in is NULL.
    virtual bool bind_consumer(i_msg_consumer_t *) = 0;
    virtual i_msg_consumer_t * bound_consumer() = 0;

    // These can be called by the consumer.
    // Get the msg.
    virtual const msg_t * get_head_msg() = 0;
    // Called by the consumer when a msg is handled.
    virtual bool msg_consumed(const msg_t *) = 0;
};

class i_msg_consumer_t {
public:
    // `msg_provided` should be called by the provider when some msg is available.
    virtual void msg_provided(i_msg_provider_t *) = 0;
};

/*******************************************************
 *
 * Component interfaces.
 *
 *******************************************************/
class i_queue_t: public i_msg_provider_t {
public:
    // Queue is universal unique.
    virtual const uuid_t * get_uuid() = 0;

    // Enqueue a message.
    virtual bool enqueue(const msg_t *) = 0;

    // When pause is called `get_head_msg` should return NULL 
    // after the last outstanding msg has been consumed even the queue is not empty.
    virtual void pause() = 0;
    virtual void resume() = 0;
};

class i_filter_t: public i_msg_consumer_t, public i_msg_provider_t {
public:
    typedef bool (*filter_fn)(const msg_t *);
    virtual void set_filter_fn(filter_fn) = 0;
};

class i_dispatcher_t: public i_consumer_t {
public:
    // Workers can be added to a dispatcher.
    // Dispatcher dispatches msg to spare worker by some strategy.
    virtual bool add_worker(i_consumer_t *) = 0;
};

// Publisher is also a consumer.
class i_publisher_t: public i_consumer_t {
public:
    virtual bool add_subscriber(i_consumer_t *) = 0;
};

}

#endif // _INTERFACE_HPP_
