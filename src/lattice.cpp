#include "lattice.h"

#include <limits>
#include <algorithm>
#include <cstdint>
#include "utils.h"

namespace mtidd
{
  
  size_t mhash(const long x) {
    //inspired by https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
    static_assert( sizeof(size_t) == 8, "expecting size_t to be 64 bits" );
    static_assert( sizeof(long) == 8, "expecting long to be 64 bits" );

    size_t h1 = 0x3141592653589793; // seed
    const uint64_t c1 = 0x87c37b91114253d5;
    const uint64_t c2 = 0x4cf5ad432745937f;

    size_t k1 = x;
    k1 *= c1;
    k1 = rotl64(k1,31);
    k1 *= c2;
    h1 ^= k1;
    h1 = rotl64(h1,27);
    h1 = h1*5+0x52dce729;

    h1 ^= 8;

    h1 ^= h1 >> 33;
    h1 *= 0xff51afd7ed558ccd;
    h1 ^= h1 >> 33;
    h1 *= 0xc4ceb9fe1a85ec53;
    h1 ^= h1 >> 33;

    return h1;
  }

  bool lattice<bool>::bottom() const { return false; }
  bool lattice<bool>::top() const { return true; }

  bool lattice<bool>::least_upper_bound(const bool& x, const bool& y) const {
    return x || y;
  }

  bool lattice<bool>::greatest_lower_bound(const bool& x, const bool& y) const {
    return x && y;
  }

  lattice_compare lattice<bool>::compare(const bool& x, const bool& y) const {
    if (x == y) {
      return Equal;
    } else if (x) {
      return Greater;
    } else {
      return Smaller;
    }
  }

  bool lattice<bool>::equal(const bool& x, const bool& y) const {
    return x == y;
  }

  // not great but good enough
  size_t lattice<bool>::hash(const bool& x) const {
    return static_cast<size_t>(x);
  }

  int lattice<int>::bottom() const {
    return std::numeric_limits<int>::min();
  }

  int lattice<int>::top() const {
    return std::numeric_limits<int>::max();
  }

  int lattice<int>::least_upper_bound(const int& x, const int& y) const {
    return std::max(x, y);
  }

  int lattice<int>::greatest_lower_bound(const int& x, const int& y) const {
    return std::min(x, y);
  }

  lattice_compare lattice<int>::compare(const int& x, const int& y) const {
    if (x == y) {
      return Equal;
    } else if (x > y) {
      return Greater;
    } else {
      return Smaller;
    }
  }

  bool lattice<int>::equal(const int& x, const int& y) const {
    return x == y;
  }

  size_t lattice<int>::hash(const int& x) const {
    return mhash(x);
  }

  long lattice<long>::bottom() const {
    return std::numeric_limits<long>::min();
  }

  long lattice<long>::top() const {
    return std::numeric_limits<long>::max();
  }

  long lattice<long>::least_upper_bound(const long& x, const long& y) const {
    return std::max(x, y);
  }

  long lattice<long>::greatest_lower_bound(const long& x, const long& y) const {
    return std::min(x, y);
  }

  lattice_compare lattice<long>::compare(const long& x, const long& y) const {
    if (x == y) {
      return Equal;
    } else if (x > y) {
      return Greater;
    } else {
      return Smaller;
    }
  }

  bool lattice<long>::equal(const long& x, const long& y) const {
    return x == y;
  }

  size_t lattice<long>::hash(const long& x) const {
    static_assert( sizeof(long) == 8 , "expecting long to be 64-bits" );
    return mhash(x);
  }

  float lattice<float>::bottom() const {
    return std::numeric_limits<float>::min();
  }

  float lattice<float>::top() const {
    return std::numeric_limits<float>::max();
  }

  float lattice<float>::least_upper_bound(const float& x, const float& y) const {
    return std::max(x, y);
  }

  float lattice<float>::greatest_lower_bound(const float& x, const float& y) const {
    return std::min(x, y);
  }

  lattice_compare lattice<float>::compare(const float& x, const float& y) const {
    if (x == y) {
      return Equal;
    } else if (x > y) {
      return Greater;
    } else {
      return Smaller;
    }
  }

  bool lattice<float>::equal(const float& x, const float& y) const {
    return x == y;
  }

  // not great but good enough
  size_t lattice<float>::hash(const float& x) const {
    double d = x;
    static_assert(sizeof(double) == sizeof(long), "expecting long and double to have the same size");
    long l = *(reinterpret_cast<long*>(&d));
    return mhash(l);
  }

  double lattice<double>::bottom() const {
    return std::numeric_limits<double>::min();
  }

  double lattice<double>::top() const {
    return std::numeric_limits<double>::max();
  }

  double lattice<double>::least_upper_bound(const double& x, const double& y) const {
    return std::max(x, y);
  }

  double lattice<double>::greatest_lower_bound(const double& x, const double& y) const {
    return std::min(x, y);
  }

  lattice_compare lattice<double>::compare(const double& x, const double& y) const {
    if (x == y) {
      return Equal;
    } else if (x > y) {
      return Greater;
    } else {
      return Smaller;
    }
  }

  bool lattice<double>::equal(const double& x, const double& y) const {
    return x == y;
  }

  size_t lattice<double>::hash(const double& x) const {
    static_assert( sizeof(double) == 8, "expecting double to be 64 bits" );
    long l = *(reinterpret_cast<const long*>(&x));
    return mhash(l);
  }

} // end namespace
