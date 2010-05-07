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

// NULL terminated string
#define NTS (0)

typedef struct msg_t {
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
class i_msg_provider_t {
public:
    /* These public interfaces are called by the scheduler */
    // Called when a msg is really consumed.
    // Return error when msgs are not consumed in order.
    virtual int on_msg_consumed(const msg_t *) = 0;

    // Called to get new msg after `ready_to_provide`.
    // The msg is not really consumed before `on_msg_consumed` is called.
    // Provider is responsable to allocate/free msg's memory.
    virtual const msg_t * get_msg() = 0;

protected:
    /* These protected interfaces are called by sub class at the right time */
    // Ready to provide new msg.
    virtual void ready_to_provide();
};


class i_msg_consumer_t {
public:
    /* These public interfaces are called by the scheduler */
    // Called when a msg is provided after `ready_to_consume`.
    virtual void on_msg_provided(const msg_t *) = 0;

protected:
    /* These protected interfaces are called by sub class at the right time */
    // Ready to consume new msg.
    virtual void ready_to_consume();

    // Called when the msg has handled.
    virtual void consume_msg(const msg_t *);
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
    virtual int enqueue(const msg_t *) = 0;
};


class i_filter_t: public i_msg_consumer_t, public i_msg_provider_t {
public:
    typedef bool (*filter_fn)(const msg_t *);
    virtual void set_filter_fn(typename filter_fn) = 0;
};


class i_publisher_t: public i_msg_consumer_t {
public:
    // Publisher is a consumer that can 'link' many consumers.
    virtual bool add_subscriber(i_msg_consumer_t *) = 0;
};

}

#endif // _INTERFACE_HPP_
