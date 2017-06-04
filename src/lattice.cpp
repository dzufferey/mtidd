#include "lattice.h"

#include <limits>
#include <algorithm>
#include <cstdint>
#include "utils.h"

namespace mtidd
{
  
  size_t mhash(const long x) {
    //inspired by https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
    static_assert( sizeof(size_t) == 8 );
    static_assert( sizeof(long) == 8 );

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

  bool boolean_lattice::bottom() const { return false; }
  bool boolean_lattice::top() const { return true; }

  bool boolean_lattice::least_upper_bound(const bool& x, const bool& y) const {
    return x || y;
  }

  bool boolean_lattice::greatest_lower_bound(const bool& x, const bool& y) const {
    return x && y;
  }

  lattice_compare boolean_lattice::compare(const bool& x, const bool& y) const {
    if (x == y) {
      return Equal;
    } else if (x) {
      return Greater;
    } else {
      return Smaller;
    }
  }

  bool boolean_lattice::equal(const bool& x, const bool& y) const {
    return x == y;
  }

  // not great but good enough
  size_t boolean_lattice::hash(const bool& x) const {
    return static_cast<size_t>(x);
  }

  int integer_lattice::bottom() const {
    return std::numeric_limits<int>::min();
  }

  int integer_lattice::top() const {
    return std::numeric_limits<int>::max();
  }

  int integer_lattice::least_upper_bound(const int& x, const int& y) const {
    return std::max(x, y);
  }

  int integer_lattice::greatest_lower_bound(const int& x, const int& y) const {
    return std::min(x, y);
  }

  lattice_compare integer_lattice::compare(const int& x, const int& y) const {
    if (x == y) {
      return Equal;
    } else if (x > y) {
      return Greater;
    } else {
      return Smaller;
    }
  }

  bool integer_lattice::equal(const int& x, const int& y) const {
    return x == y;
  }

  size_t integer_lattice::hash(const int& x) const {
    return mhash(x);
  }

  long long_lattice::bottom() const {
    return std::numeric_limits<long>::min();
  }

  long long_lattice::top() const {
    return std::numeric_limits<long>::max();
  }

  long long_lattice::least_upper_bound(const long& x, const long& y) const {
    return std::max(x, y);
  }

  long long_lattice::greatest_lower_bound(const long& x, const long& y) const {
    return std::min(x, y);
  }

  lattice_compare long_lattice::compare(const long& x, const long& y) const {
    if (x == y) {
      return Equal;
    } else if (x > y) {
      return Greater;
    } else {
      return Smaller;
    }
  }

  bool long_lattice::equal(const long& x, const long& y) const {
    return x == y;
  }

  size_t long_lattice::hash(const long& x) const {
    static_assert( sizeof(long) == 8 );
    return mhash(x);
  }

  float float_lattice::bottom() const {
    return std::numeric_limits<float>::min();
  }

  float float_lattice::top() const {
    return std::numeric_limits<float>::max();
  }

  float float_lattice::least_upper_bound(const float& x, const float& y) const {
    return std::max(x, y);
  }

  float float_lattice::greatest_lower_bound(const float& x, const float& y) const {
    return std::min(x, y);
  }

  lattice_compare float_lattice::compare(const float& x, const float& y) const {
    if (x == y) {
      return Equal;
    } else if (x > y) {
      return Greater;
    } else {
      return Smaller;
    }
  }

  bool float_lattice::equal(const float& x, const float& y) const {
    return x == y;
  }

  // not great but good enough
  size_t float_lattice::hash(const float& x) const {
    double d = x;
    static_assert(sizeof(double) == sizeof(long));
    long l = *(reinterpret_cast<long*>(&d));
    return mhash(l);
  }

  double double_lattice::bottom() const {
    return std::numeric_limits<double>::min();
  }

  double double_lattice::top() const {
    return std::numeric_limits<double>::max();
  }

  double double_lattice::least_upper_bound(const double& x, const double& y) const {
    return std::max(x, y);
  }

  double double_lattice::greatest_lower_bound(const double& x, const double& y) const {
    return std::min(x, y);
  }

  lattice_compare double_lattice::compare(const double& x, const double& y) const {
    if (x == y) {
      return Equal;
    } else if (x > y) {
      return Greater;
    } else {
      return Smaller;
    }
  }

  bool double_lattice::equal(const double& x, const double& y) const {
    return x == y;
  }

  size_t double_lattice::hash(const double& x) const {
    static_assert( sizeof(double) == 8 );
    long l = *(reinterpret_cast<const long*>(&x));
    return mhash(l);
  }

} // end namespace
