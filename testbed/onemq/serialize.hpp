#ifndef _SERIALIZE_HPP_
#define _SERIALIZE_HPP_

#include <stdint.h>

namespace omq {

//Convert to big-endian for network transfer and disk-store.
void serialize(uint16_t in, unsigned char * out);
void serialize(uint32_t in, unsigned char * out);
void serialize(uint64_t in, unsigned char * out);

void unserialize(const unsigned char * in, uint16_t * out);
void unserialize(const unsigned char * in, uint32_t * out);
void unserialize(const unsigned char * in, uint64_t * out);

template <typename T>
struct buff_t {
    typedef unsigned char type[sizeof(T)];
};

}

#endif // _SERIALIZE_HPP_
