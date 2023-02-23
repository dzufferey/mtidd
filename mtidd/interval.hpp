#pragma once

#include <iostream>
#include <limits>
#include <tuple>
#include <compare>

namespace mtidd {

///////////////
// boudaries //
///////////////

typedef enum { Open, Closed } interval_boundary;

interval_boundary complement(interval_boundary b);

std::ostream &operator<<(std::ostream &out, const interval_boundary &b);

///////////////////
// half interval //
///////////////////

typedef std::tuple<double, interval_boundary> half_interval;

// lhs is the right/end of an interval
// rhs is the left/start of an interval
bool ends_before(const half_interval &lhs, const half_interval &rhs);

// lhs and rhs are the right/end of an interval
std::partial_ordering operator<=>(const half_interval &lhs, const half_interval &rhs);

bool contains(const half_interval &end, double value);

// lhs is the right/end of an interval
// rhs is the left/start of an interval
const half_interval &min(const half_interval &lhs, const half_interval &rhs);

// i is the right/end of an interval
half_interval complement(const half_interval &i);

std::ostream &operator<<(std::ostream &out, const half_interval &i);

// sentinel nodes
const half_interval lower_sentinel =
    std::make_tuple<double, interval_boundary>(-std::numeric_limits<double>::infinity(), Open);
const half_interval upper_sentinel =
    std::make_tuple<double, interval_boundary>(std::numeric_limits<double>::infinity(), Open);

//////////////
// interval //
//////////////

typedef std::tuple<double, interval_boundary, double, interval_boundary> interval;

interval make_interval(const half_interval &lb, const half_interval &ub);

// lb and ub are the right/end of an interval
interval make_interval_after(const half_interval &lb, const half_interval &ub);

bool is_empty(const interval &i);

bool is_singleton(const interval &i);

bool contains(const interval &i, double value);

bool overlap(const interval &i1, const interval &i2);

bool covers(const interval &covering, const interval &covered);

// returns the half_interval ending just before i, i starts after the returned interval
half_interval starts_after(const interval &i);

half_interval ends(const interval &i);

std::ostream &operator<<(std::ostream &out, const interval &i);

} // namespace mtidd
