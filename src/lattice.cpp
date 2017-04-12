#include "lattice.h"

#include <limits>
#include <algorithm>

namespace mtidd
{
  
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

} // end namespace
