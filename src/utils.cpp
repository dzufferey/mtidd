#include "utils.h"

namespace mtidd
{
  
  uint64_t rotl64 ( uint64_t x, int8_t r )
  {
        return (x << r) | (x >> (64 - r));
  }

}
