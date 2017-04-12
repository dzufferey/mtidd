#include "partition.h"

using namespace std;

namespace mtidd
{
  
  interval_boundary complement(interval_boundary b) {
    return v == Open ? Closed : Open;
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
               get<1>(rhs) == Closed );
  }

}
