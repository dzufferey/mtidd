#include "lattice.h"

#include <limits>
#include <algorithm>
#include <cstring>

#include "utils.h"

namespace mtidd
{

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
    static_assert(sizeof(double) == sizeof(long), "expecting long and double to have the same size");
    double d = x;
    long l;
    memcpy(&l, &d, sizeof(l)); // this is needed to go around strict-aliasing rules
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
