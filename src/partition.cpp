#include "partition.h"

using namespace std;

namespace mtidd
{
  
  interval_boundary complement(interval_boundary b) {
    return b == Open ? Closed : Open;
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
             ( get<1>(lhs) == get<1>(rhs) ||
               get<1>(rhs) == Closed )
           );
  }

  bool contains(const half_interval& end, double value) {
    double v = get<0>(end);
    return value < v || (value == v && get<1>(end) == Closed);
  }

  const half_interval& min(const half_interval& lhs, const half_interval& rhs) {
    return ends_before(lhs, rhs) ? lhs : rhs;
  }


}
