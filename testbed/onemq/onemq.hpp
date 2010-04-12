#ifndef _ONEMQ_HPP_
#define _ONEMQ_HPP_

namespace omq {

class Queue;
class QueueReader;
typedef struct MessageID MessageID;
typedef struct Message Message;


class _QActor {
public:
    int last_err() { return _last_err; }
protected:
    void set_last_err(int err) { _last_err = err; }
private:
    int _last_err;
};


struct Message {
    const char * content;
    size_t len;
    MessageID id;
};


class Queue {
public:
    virtual MsgProducer * create_producer() = 0;
    virtual MsgConsumer * create_consumer() = 0;
    virtual QueueReader * create_reader() = 0;
};


// XXX !!! need to think about how external transactions
// to avoid duplicate produce
class MsgProducer: public _QActor {
public:
    virtual bool produce(Message * msg) = 0;
};


// XXX !!! conflict with the reader
class MsgConsumer: public _QActor {
public:
    virtual bool consume(Message * msg) = 0;
};


class QueueReader: public _QActor {
public:
    // Seek to some position in the message queue.
    // Return false if seek to an unvalid position.
    virtual bool seek(const MessageID * mid) = 0;

    // Read the next msg. Message is a block of opaque memory.
    // If `block` == false, the function will return (false) immediately if 
    // there is no more messages in the queue now.
    // Note that the `content` holded in `msg` is ONLY valid before the next read.
    // Don't free it yourself.
    virtual bool next(Message * msg, bool block = true) = 0;
};


}

#endif // _ONEMQ_HPP_
