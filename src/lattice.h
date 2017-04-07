#include <limits>
#include <algorithm>

namespace mtidd
{

  // comparison in a partial ordering
  typedef enum { Equal, Smaller, Greater, Different} lattice_compare;

  // struct that contains the methods to
  template<typename T> struct lattice {
    const T bottom;
    const T top;

    T least_upper_bound(const T&, const T&) const;
    T greatest_lower_bound(const T&, const T&) const;

    lattice_compare compare(const T&, const T&) const;

    bool operator==(const T&, const T&) const;
    bool operator!=(const T&, const T&) const;
  };



  // Simple instances of Lattices

  template<> struct lattice<bool> {
    const bool bottom = false;
    const bool top = true;

    bool least_upper_bound(const bool& x, const bool& y) const {
      return x || y;
    }

    bool greatest_lower_bound(const bool& x, const bool& y) const {
      return x && y;
    }

    lattice_compare compare(const bool& x, const bool& y) const {
      if (x == y) {
        return Equal;
      } else if (x) {
        return Greater;
      } else {
        return Smaller;
      }
    }

    bool operator==(const bool& x, const bool& y) const {
      return x == y;
    }

    bool operator!=(const bool& x, const bool& y) const {
      return x != y;
    }
  };

  template<> struct lattice<int> {
    const int bottom = std::numeric_limits<int>::min();
    const int top = std::numeric_limits<int>::max();

    int least_upper_bound(const int& x, const int& y) const {
      return std::max(x, y);
    }

    int greatest_lower_bound(const int& x, const int& y) const {
      return std::min(x, y);
    }

    lattice_compare compare(const int& x, const int& y) const {
      if (x == y) {
        return Equal;
      } else if (x > y) {
        return Greater;
      } else {
        return Smaller;
      }
    }

    bool operator==(const int& x, const int& y) const {
      return x == y;
    }

    bool operator!=(const int& x, const int& y) const {
      return x != y;
    }
  };

  template<> struct lattice<long> {
    const long bottom = std::numeric_limits<long>::min();
    const long top = std::numeric_limits<long>::max();

    long least_upper_bound(const long& x, const long& y) const {
      return std::max(x, y);
    }

    long greatest_lower_bound(const long& x, const long& y) const {
      return std::min(x, y);
    }

    lattice_compare compare(const long& x, const long& y) const {
      if (x == y) {
        return Equal;
      } else if (x > y) {
        return Greater;
      } else {
        return Smaller;
      }
    }

    bool operator==(const long& x, const long& y) const {
      return x == y;
    }

    bool operator!=(const long& x, const long& y) const {
      return x != y;
    }
  };

  template<> struct lattice<float> {
    const float bottom = std::numeric_limits<float>::min();
    const float top = std::numeric_limits<float>::max();

    float least_upper_bound(const float& x, const float& y) const {
      return std::max(x, y);
    }

    float greatest_lower_bound(const float& x, const float& y) const {
      return std::min(x, y);
    }

    lattice_compare compare(const float& x, const float& y) const {
      if (x == y) {
        return Equal;
      } else if (x > y) {
        return Greater;
      } else {
        return Smaller;
      }
    }

    bool operator==(const float& x, const float& y) const {
      return x == y;
    }

    bool operator!=(const float& x, const float& y) const {
      return x != y;
    }
  };

  template struct lattice<double> {
    const double bottom = std::numeric_limits<double>::min();
    const double top = std::numeric_limits<double>::max();

    double least_upper_bound(const double& x, const double& y) const {
      return std::max(x, y);
    }

    double greatest_lower_bound(const double& x, const double& y) const {
      return std::min(x, y);
    }

    lattice_compare compare(const double& x, const double& y) const {
      if (x == y) {
        return Equal;
      } else if (x > y) {
        return Greater;
      } else {
        return Smaller;
      }
    }

    bool operator==(const double& x, const double& y) const {
      return x == y;
    }

    bool operator!=(const double& x, const double& y) const {
      return x != y;
    }
  };

  // TODO an example for set<T>

} // end namespace
