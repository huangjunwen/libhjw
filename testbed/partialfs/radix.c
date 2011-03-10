#include <stdint.h>
#include "radix.h"

// _get_bit("ab", 0..7) -> 10000110 (97)
unsigned char _get_bit(const char * s, size_t bitlen, int bitidx) {
    return (bitidx < 0 || bitidx >= bitlen) ? 0 :
        (s[bitidx >> 3] & (1 << (bitidx & 15)));
}

int _diff_bitidx(const char * s1, const char * s2) {
    // ref:
    //  http://www.matrix67.com/blog/archives/3985
    //  http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
    //  http://en.wikipedia.org/wiki/De_Bruijn_sequence
    // we need 8-bits seq, how to get it:
    // B(2, 3): 00010111   (0x17)
    //   000, 001, 010, 101, 011, 111, 110, 100
    //   0, 1, 2, 5, 3, 7, 6, 4
    //   m[0] = 0, m[1] = 1, m[2] = 2, m[5] = 3,
    //   m[3] = 4, m[7] = 5, m[6] = 6, m[4] = 7
    static const int _DeBruijinBitPos[8] = {
        0, 1, 2, 4, 7, 3, 6, 5
    };
    const char * p1, * p2;
    int8_t c;
    int r;

    p1 = s1;
    p2 = s2;
    r = 0;
    while (*p1 == *p2) {
        ++r;
        ++p1;
        ++p2;
    }
    r <<= 3;                // *= 8

    c = (*p1) ^ (*p2);      // xor, so the first diff bit will the
                            // first 1 from the low end
    return r + _DeBruijinBitPos[((uint8_t)((c & -c) * 0x17)) >> 5];
}
