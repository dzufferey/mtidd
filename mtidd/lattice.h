#pragma once

#include <cstddef>
#include <cstdint>

#include "utils.h"

namespace mtidd
{

  extern size_t mhash(const long x);

  // comparison in a partial ordering
  typedef enum { Equal, Smaller, Greater, Different} lattice_compare;

  //TODO could we use concept to get rid of the virtual part

  // struct that contains the methods to
  template<typename T>
  struct lattice {
    T bottom() const;
    T top() const;

    T least_upper_bound(const T&, const T&) const;
    T greatest_lower_bound(const T&, const T&) const;

    lattice_compare compare(const T&, const T&) const;
    bool equal(const T&, const T&) const;
    size_t hash(const T&) const;
  };

  // Simple instances of Lattices
  template<>
  struct lattice<bool> {
  public:
    bool bottom() const;
    bool top() const;
    bool least_upper_bound(const bool& x, const bool& y) const;
    bool greatest_lower_bound(const bool& x, const bool& y) const;
    lattice_compare compare(const bool& x, const bool& y) const;
    bool equal(const bool& x, const bool& y) const;
    size_t hash(const bool& x) const;
  };

  template<>
  struct lattice<int> {
  public:
    int bottom() const;
    int top() const;
    int least_upper_bound(const int& x, const int& y) const;
    int greatest_lower_bound(const int& x, const int& y) const;
    lattice_compare compare(const int& x, const int& y) const;
    bool equal(const int& x, const int& y) const;
    size_t hash(const int& x) const;
  };

  template<>
  struct lattice<long> {
  public:
    long bottom() const;
    long top() const;
    long least_upper_bound(const long& x, const long& y) const;
    long greatest_lower_bound(const long& x, const long& y) const;
    lattice_compare compare(const long& x, const long& y) const;
    bool equal(const long& x, const long& y) const;
    size_t hash(const long& x) const;
  };

  template<>
  struct lattice<float> {
  public:
    float bottom() const;
    float top() const;
    float least_upper_bound(const float& x, const float& y) const;
    float greatest_lower_bound(const float& x, const float& y) const;
    lattice_compare compare(const float& x, const float& y) const;
    bool equal(const float& x, const float& y) const;
    size_t hash(const float& x) const;
  };

  template<>
  struct lattice<double> {
  public:
    double bottom() const;
    double top() const;
    double least_upper_bound(const double& x, const double& y) const;
    double greatest_lower_bound(const double& x, const double& y) const;
    lattice_compare compare(const double& x, const double& y) const;
    bool equal(const double& x, const double& y) const;
    size_t hash(const double& x) const;
  };

  // TODO an example for set<T>

} // end namespace
