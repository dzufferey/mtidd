#include "utils.h"

namespace mtidd
{
  
  uint64_t rotl64 ( uint64_t x, int8_t r )
  {
    return (x << r) | (x >> (64 - r));
  }

  uint64_t combine_two_hashes( uint64_t x, uint64_t y )
  {
    return (x << 9) ^ (y >> 7);
  }

}
