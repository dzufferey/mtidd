#pragma once

#include <tuple>
#include <limits>

using namespace std;

namespace mtidd
{

  ///////////////
  // boudaries //
  ///////////////

  typedef enum { Open, Closed } interval_boundary;

  interval_boundary complement(interval_boundary b);

  ///////////////////
  // half interval //
  ///////////////////

  typedef tuple<double,interval_boundary> half_interval;

  // lhs is the right/end of an interval
  // rhs is the left/start of an interval
  bool ends_before(const half_interval& lhs, const half_interval& rhs);

  // lhs and rhs are the right/end of an interval
  bool operator<=(const half_interval& lhs, const half_interval& rhs);

  bool contains(const half_interval& end, double value);

  // lhs is the right/end of an interval
  // rhs is the left/start of an interval
  const half_interval& min(const half_interval& lhs, const half_interval& rhs);

  // sentinel nodes
  constexpr half_interval lower_sentinel = make_tuple<double, interval_boundary>(-numeric_limits<double>::infinity(), Open);
  constexpr half_interval upper_sentinel = make_tuple<double, interval_boundary>( numeric_limits<double>::infinity(), Open);

  //////////////
  // interval //
  //////////////

  typedef tuple<double,interval_boundary,double,interval_boundary> interval;

  bool is_empty(const interval& i);

  bool is_singleton(const interval& i);

  bool contains(const interval& i, double value);

  half_interval starts_after(const interval& i);

  half_interval ends(const interval& i);

}
