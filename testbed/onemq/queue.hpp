#ifndef _QUEUE_HPP_
#define _QUEUE_HPP_

#include <stdint.h>
#include <uuid/uuid.h>

namespace omq {

/**************************
 *          Msg           *
 **************************/
typedef struct msg_id_t {
    msg_id_t(): src(NULL), seq(0) {}
    const uuid_t * src;                                 // uuid of the msg source
    int64_t seq;                                        // monotonic increasing number
} msg_id_t;

typedef struct msg_t {
    msg_t(): content(NULL), len(0), id() {}
    const char * content;
    size_t len;
    msg_id_t id;
} msg_t;

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

    // A queue can create 0+ producers (depends on what kind of queue is and
    // status of the queue).
    virtual producer_t * create_producer() = 0;

    // A queue can create 0+ consumers (depends on what kind of queue is and
    // status of the queue).
    virtual consumer_t * create_consumer() = 0;

protected:
    uuid_t _id;
};

/**************************
 * Producer and Consumer  *
 **************************/
typedef enum {
    MSG_TOO_OLD = 1,                                // message is too old (already sent)
    UNKNOWN = 255                                   // unknown error
} err_t;

class _queue_actor {
public:
    err_t last_err() { return _last_err; }
protected:
    err_t _last_err;
};

class producer_t: public _queue_actor {
public:
    // Enqueue a message.
    // `msg` can be from some external source so its `id` is not 0.
    // In this case `producer_t` MUST check whether it is newer than the most recent 
    // one from the source.
    // Pass `seq` in if interested in the new seq in this queue.
    virtual bool enqueue(const msg_t * msg, uint64_t * seq) = 0;
};

class consumer_t: public _queue_actor {
public:
    // Read the 'head' msg.
    // Note that the `content` holded in `msg` is ONLY valid before the next read.
    // Don't free it yourself.
    virtual bool read(msg_t * msg) = 0;

    // Dequeue 'head' msg.
    virtual bool dequeue(msg_t * msg) = 0;
};


}

#endif // _QUEUE_HPP_
