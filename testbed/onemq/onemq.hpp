#ifndef _ONEMQ_HPP_
#define _ONEMQ_HPP_

#include <stdint.h>
#include <uuid/uuid.h>

namespace omq {

/**************************
 *          Msg           *
 **************************/
#define UNKNOWN_SEQ (UINT64_MAX-1)

typedef struct {
    const uuid_t * src;             // uuid of the msg source
    int64_t seq;                    // monotonic increasing number
} msg_id_t;

typedef struct {
    const char * content;
    size_t len;
} msg_t;

/**************************
 *          Queue         *
 **************************/
class producer_t;
class consumer_t;

class queue_t {
public:
    // A queue is universal unique.
    queue_t(const uuid_t id);
    const uuid_t * get_id();

    // A queue can create multiple producers.
    virtual producer_t * create_producer() = 0;

    // A queue can create one consumer only.
    virtual consumer_t * create_consumer() = 0;

private:
    uuid_t _id;
};

/**************************
 * Producer and Consumer  *
 **************************/
typedef enum {
    MSG_TOO_OLD = 1,                // message is too old (already sent)
    UNKNOWN = 255                   // unknown error
} err_t;

class _queue_actor {
public:
    err_t last_err() { return _last_err; }
protected:
    void set_last_err(err_t err) { _last_err = err; }
private:
    err_t _last_err;
};

class producer_t: public _queue_actor {
public:
    // Enqueue a message.
    // If the message has an orignal source, pass msg_id in.
    // Return seq number in this queue or UNKNOWN_SEQ on error.
    virtual uint64_t enqueue(const msg_id_t * msg_id, const msg_t * msg) = 0;
};

class consumer_t: public _queue_actor {
public:
    // Read the 'head' msg.
    // Note that the `content` holded in `msg` is ONLY valid before the next read.
    // Don't free it yourself.
    virtual bool read(msg_id_t * msg_id, msg_t * msg) = 0;

    // Dequeue 'head' msg.
    virtual bool dequeue(msg_id_t * msg_id, msg_t * msg) = 0;
};


}

#endif // _ONEMQ_HPP_
