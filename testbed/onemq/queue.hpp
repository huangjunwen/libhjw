#ifndef _QUEUE_HPP_
#define _QUEUE_HPP_

#include <stdint.h>
#include <uuid/uuid.h>

namespace omq {

/**************************
 *          Msg           *
 **************************/
#define NULL_SEQ (0)

typedef struct msg_id_t {
    msg_id_t(): src(NULL), seq(0) {}

    // UUID of the msg source.
    const uuid_t * src;                                 

    // Monotonic increasing number start from 1. 
    // 0 is reserved.
    // Seq maybe not continuous. (filter)
    uint64_t seq;                                       
} msg_id_t;

class msg_t {
public:
    msg_id_t id;

    // Return the total len of the content.
    virtual size_t get_content_len() = 0;

    // Return next block of content.
    virtual size_t get_content(const unsigned char ** content) = 0;
};

/**************************
 *          Queue         *
 **************************/
class producer_t;
class consumer_t;

class queue_t {
public:
    // A queue is identified by an uuid.
    queue_t();
    bool set_id(const uuid_t id);
    const uuid_t * get_id();

    // A queue can create 1+ producers (depends on what kind of queue is and
    // status of the queue).
    virtual producer_t * create_producer() = 0;

    // A queue can create only one consumer (single thread fashion).
    virtual consumer_t * create_consumer() = 0;

protected:
    uuid_t id;
};

/**************************
 * Producer and Consumer  *
 **************************/
typedef enum {
    MSG_TOO_OLD = 1,                                // message is too old (already sent)
    UNKNOWN = 255                                   // unknown error
} err_t;

class _queue_actor_t {
public:
    err_t get_last_err() { return last_err; }
protected:
    err_t last_err;
};

class producer_t: public _queue_actor_t {
public:
    // Enqueue a message.
    // `msg` can be from some external source so its `id` is not 0.
    // In this case `producer_t` MUST check whether it is newer than the most recent 
    // one from the source.
    virtual bool enqueue(const msg_t * msg) = 0;
};

class consumer_t: public _queue_actor_t {
public:
    // Read the 'head' msg.
    // Note that the `content` holded in `msg` is ONLY valid before the next read.
    // Don't free it yourself.
    // Return false if no more to read.
    virtual bool read(msg_t * msg) = 0;

    // Dequeue 'head' msg.
    virtual bool dequeue(msg_t * msg) = 0;
};


}

#endif // _QUEUE_HPP_
