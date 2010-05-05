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
    msg_id_t id;
    const unsigned char * content;
    size_t len;
} msg_t;

/*******************************************************
 *
 * Component interfaces.
 * Queue is responsable to provide msg.
 * Consumer is responsable to handle msg. 
 *
 *******************************************************/
class i_queue_t;
class i_consumer_t;

class i_queue_t {
public:
    // Queue is universal unique.
    virtual const uuid_t * get_uuid() = 0;

    // Enqueue a message.
    virtual bool enqueue(const msg_t *) = 0;

    // A queue can be pause and resume.
    virtual void pause() = 0;
    virtual void resume() = 0;
    // Return the number of messages that have given to the consumer but 
    // not consumed.
    virtual uint32_t outstand_msgs() = 0;

    // A provider can bind ONLY ONE consumer. 
    virtual bool bind_consumer(i_consumer_t *) = 0;
    virtual i_consumer_t * bound_consumer() = 0;

    // These can be called by the consumer.
    // Get the msg.
    virtual const msg_t * get_msg(const msg_id_t *) = 0;
    // Called by the consumer when a msg is handled.
    virtual bool msg_consumed(const msg_id_t *) = 0;
};

class i_consumer_t {
public:
    // `msg_provided` should be called by the provider when some msg is available.
    virtual void msg_provided(i_queue_t *, const msg_id_t *) = 0;
};

// Publisher is also a consumer.
class i_publisher_t: public i_consumer_t {
public:
    virtual bool add_subscriber(i_consumer_t *) = 0;
};

}

#endif // _INTERFACE_HPP_
