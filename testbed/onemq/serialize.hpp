#ifndef _SERIALIZE_HPP_
#define _SERIALIZE_HPP_

#include <stdint.h>

namespace omq {

template <typename T>
struct buff_t {
    typedef unsigned char type[sizeof(T)];
};

//Convert to big-endian for network transfer and disk-store.
void serialize(int16_t in, unsigned char * out);
void serialize(uint16_t in, unsigned char * out);
void serialize(int32_t in, unsigned char * out);
void serialize(uint32_t in, unsigned char * out);
void serialize(int64_t in, unsigned char * out);
void serialize(uint64_t in, unsigned char * out);

void unserialize(int16_t * out, const unsigned char * in);
void unserialize(uint16_t * out, const unsigned char * in);
void unserialize(int32_t * out, const unsigned char * in);
void unserialize(uint32_t * out, const unsigned char * in);
void unserialize(int64_t * out, const unsigned char * in);
void unserialize(uint64_t * out, const unsigned char * in);

}

#endif // _SERIALIZE_HPP_
