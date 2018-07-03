#pragma once

#include <cstdint>
#include <functional>

namespace mtidd
{

  uint64_t rotl64 ( uint64_t x, int8_t r );

  uint64_t combine_two_hashes( uint64_t x, uint64_t y );
 
  size_t mhash(const long x);

  template< typename T, typename Hash = std::hash<T> >
  struct pair_hash {
    size_t operator()(std::tuple<T, T> const & s) const
    {
      return combine_two_hashes( Hash{}(std::get<0>(s)), Hash{}(std::get<1>(s)));
    }
  };

}
