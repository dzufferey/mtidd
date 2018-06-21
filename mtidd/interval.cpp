#include "interval.h"
#include <cassert>

using namespace std;

namespace mtidd
{
  
  interval_boundary complement(interval_boundary b) {
    return b == Open ? Closed : Open;
  }
  
  ostream & operator<<(ostream & out, const interval_boundary& b) {
    if (b == Open) {
      return out << "Open";
    } else {
      return out << "Closed";
    }
  }

  // lhs is the right/end of an interval
  // rhs is the left/start of an interval
  bool ends_before(const half_interval& lhs, const half_interval& rhs) {
    double left = get<0>(lhs);
    double right = get<0>(rhs);
    return left < right ||
           (left == right && get<1>(lhs) == Open);
  }

  // lhs and rhs are the right/end of an interval
  bool operator<=(const half_interval& lhs, const half_interval& rhs) {
    double left = get<0>(lhs);
    double right = get<0>(rhs);
    return left < right ||
           ( left == right &&
             ( get<1>(lhs) == Open ||
               get<1>(rhs) == Closed )
           );
  }

  // lhs and rhs are the right/end of an interval
  bool operator<(const half_interval& lhs, const half_interval& rhs) {
    double left = get<0>(lhs);
    double right = get<0>(rhs);
    return left < right ||
           ( left == right &&
             get<1>(lhs) == Open &&
             get<1>(rhs) == Closed
           );
  }

  bool contains(const half_interval& end, double value) {
    double v = get<0>(end);
    return value < v || (value == v && get<1>(end) == Closed);
  }

  const half_interval& min(const half_interval& lhs, const half_interval& rhs) {
    return ends_before(lhs, rhs) ? lhs : rhs;
  }
  
  // i is the right/end of an interval
  half_interval complement(const half_interval& i) {
    double value = get<0>(i);
    assert(value != numeric_limits<double>::infinity());
    if (value == -numeric_limits<double>::infinity()) {
      return make_tuple(value, Open);
    } else {
      return make_tuple(value, complement(get<1>(i)));
    }
  }
  
  ostream & operator<<(ostream & out, const half_interval& i) {
    if (get<1>(i) == Open) {
      return out << get<0>(i) << ")";
    } else {
      return out << get<0>(i) << "]";
    }
  }
  
  interval make_interval(const half_interval& lb, const half_interval& ub) {
    return make_tuple(get<0>(lb), get<1>(lb), get<0>(ub), get<1>(ub));
  }

  // lb and ub are the right/end of an interval
  interval make_interval_after(const half_interval& lb, const half_interval& ub) {
    half_interval new_lb = complement(lb);
    return make_interval(new_lb, ub);
  }

  bool is_empty(const interval& i) {
    return get<0>(i) > get<2>(i) ||
           (get<0>(i) == get<2>(i) && (get<1>(i) == Open || get<3>(i) == Open));
  }

  bool is_singleton(const interval& i) {
    return get<0>(i) == get<2>(i) && get<1>(i) == Closed && get<3>(i) == Closed;
  }

  bool contains(const interval& i, double value) {
    return (value > get<0>(i) || (value == get<0>(i) && get<1>(i) == Closed)) &&
           (value < get<2>(i) || (value == get<2>(i) && get<3>(i) == Closed));
  }
  
  bool overlap(const interval& i1, const interval& i2) {
    return !is_empty(i1) && !is_empty(i2) && (
      (starts_after(i1) <= starts_after(i2) && starts_after(i2) < ends(i1)) ||
      (starts_after(i2) <= starts_after(i1) && starts_after(i1) < ends(i2))
    );
  }

  bool covers(const interval& covering, const interval& covered) {
    return !is_empty(covering) && !is_empty(covered) && 
           starts_after(covering) <= starts_after(covered) &&
           ends(covered) <= ends(covering);
  }
  
  half_interval starts_after(const interval& i) {
    double value = get<0>(i);
    assert(value != numeric_limits<double>::infinity());
    return make_tuple(value, complement(get<1>(i)));
  }

  half_interval ends(const interval& i) {
    return make_tuple(get<2>(i), get<3>(i));
  }

  ostream & operator<<(ostream & out, const interval& i) {
    if (get<1>(i) == Open) {
      out << "(";
    } else {
      out << "[";
    }
    out << get<0>(i) << ";" << get<2>(i);
    if (get<3>(i) == Open) {
      return out << ")";
    } else {
      return out << "]";
    }
  }

}
