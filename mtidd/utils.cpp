#include "utils.hpp"

namespace mtidd {

uint64_t rotl64(uint64_t x, int8_t r) { return (x << r) | (x >> (64 - r)); }

uint64_t combine_two_hashes(uint64_t x, uint64_t y) { return (x << 9) ^ (y >> 7); }

size_t mhash(const long x) {
    // inspired by https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
    static_assert(sizeof(size_t) == 8, "expecting size_t to be 64 bits");
    static_assert(sizeof(long) == 8, "expecting long to be 64 bits");

    size_t h1 = 0x3141592653589793; // seed
    const uint64_t c1 = 0x87c37b91114253d5;
    const uint64_t c2 = 0x4cf5ad432745937f;

    size_t k1 = x;
    k1 *= c1;
    k1 = rotl64(k1, 31);
    k1 *= c2;
    h1 ^= k1;
    h1 = rotl64(h1, 27);
    h1 = h1 * 5 + 0x52dce729;

    h1 ^= 8;

    h1 ^= h1 >> 33;
    h1 *= 0xff51afd7ed558ccd;
    h1 ^= h1 >> 33;
    h1 *= 0xc4ceb9fe1a85ec53;
    h1 ^= h1 >> 33;

    return h1;
}

} // namespace mtidd
