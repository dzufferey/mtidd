#pragma once

#include <cstdint>

using namespace std;

namespace mtidd
{
  
  uint64_t rotl64 ( uint64_t x, int8_t r );

  uint64_t combine_two_hashes( uint64_t x, uint64_t y );

}
