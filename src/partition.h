#pragma once

#include <assert.h>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <tuple>

#include "interval.h"
#include "utils.h"

// represent partitions with DLL of open/closed boundaries over double

namespace mtidd
{

  // a partition is just an alias for a list of boundaries
  template<class A>
  using partition = std::list<std::tuple<half_interval, const A *>>;

  template<class A>
  partition<A> new_partition(const A * default_value) {
    partition<A> boundaries;
    boundaries.emplace_back(upper_sentinel, default_value);
    return boundaries;
  }

  // insert an interval into a partition
  template<class A>
  void insert_partition(partition<A>& boundaries, interval const& i, const A * value) {
    //cerr << "inserting " << i << " in " << boundaries << endl;
    assert(!is_empty(i));
    half_interval new_start = std::make_tuple(std::get<0>(i), std::get<1>(i));
    auto iterator = boundaries.begin();
    // find where to start
    bool gap = true; // we only need to insert a new boundary when there is a gap with the previous one
    while ( ends_before(std::get<0>(*iterator), new_start) ) {
      gap = std::get<0>(std::get<0>(*iterator)) < std::get<0>(new_start);
      ++iterator;
    }
    // insert new boundary if needed
    auto next_stop = std::get<0>(*iterator);
    //cerr << "inserting at/before " << next_stop << endl;
    bool overlap = gap && (std::get<0>(next_stop) > std::get<0>(new_start) || (std::get<1>(next_stop) == Closed && std::get<1>(new_start) == Open));
    // overlap && old_value != new_value
    if ( overlap && std::get<1>(*iterator) != value ) {
       // insert new stop with value of old_stop
       half_interval complement_of_new = std::make_tuple(std::get<0>(new_start), complement(std::get<1>(new_start)));
       boundaries.emplace(iterator, complement_of_new, std::get<1>(*iterator));
    }
    // find where to stop
    half_interval new_stop = std::make_tuple(std::get<2>(i), std::get<3>(i));
    while ( std::get<0>(*iterator) <= new_stop && iterator != boundaries.end() ) {
      auto to_remove = iterator;
      ++iterator;
      boundaries.erase(to_remove);
    }
    //cerr << "stoping at/before " << std::get<0>(*iterator) << endl;
    // insert new boundary if needed
    if ( iterator == boundaries.end() || std::get<1>(*iterator) != value ) {
      boundaries.emplace(iterator, new_stop, value);
    }
  }

  template<class A>
  void map_partition(partition<A>& result, partition<A> const& arg, std::function<const A* (const A *)> map_elements) {
    const A * previous_value = nullptr;
    result.clear();
    auto iterator = arg.begin();
    while (iterator != arg.end()){

      const half_interval& next = std::get<0>(*iterator);
      const A * next_value = map_elements(std::get<1>(*iterator));
      assert(next_value != nullptr);

      // merge previous and next if they point to the same value
      if (next_value == previous_value) {
        result.pop_back();
      }
      previous_value = next_value;

      result.emplace_back(next, next_value);
      iterator++;
    }
  }

  template<class A>
  void filter_partition(std::list<interval>& result, partition<A> const& arg, std::function<bool (const A *)> filter_elements) {
    result.clear();
    auto iterator = arg.begin();
    half_interval lhs = lower_sentinel;
    half_interval rhs = std::get<0>(*iterator);
    while (iterator != arg.end()) {
      if (filter_elements(std::get<1>(*iterator))) {
          interval intv = std::make_tuple(std::get<0>(lhs), std::get<1>(lhs), std::get<0>(rhs), std::get<1>(rhs));
          result.push_back(intv);
      }
      iterator++;
      lhs = std::make_tuple(std::get<0>(rhs), complement(std::get<1>(rhs)));
      rhs = std::get<0>(*iterator);
    }
  }

  template<class A, class B>
  void foldl_partition(B& result, B init, partition<A> const& arg, std::function<B (const B, const A*)> combine) {
    result = init;
    auto iterator = arg.begin();
    while (iterator != arg.end()) {
      result = combine(result, std::get<1>(*iterator));
      iterator++;
    }
  }

  template <class A>
  bool covering_sat(partition<A> const& arg, interval& intv, A& data, std::function<bool (const A *, const A)> sat) {
    std::list<const A*> contents;
    interval_covered_by(contents, arg, data);
    for (auto iterator = contents.begin(); iterator != contents.end(); iterator++) {
        if (!sat(*iterator, data)) return false;
    }
    return true;
  }

  //a method to merge to partition and combine the values
  //the result is stored into `result`
  template<class A>
  void merge_partition(partition<A>& result, partition<A> const& lhs, partition<A> const& rhs, std::function<const A* (const A *, const A *)> merge_elements) {

    const A * previous_value = nullptr;
    result.clear();

    auto iterator_lhs = lhs.begin();
    auto iterator_rhs = rhs.begin();

    while (iterator_lhs != lhs.end() ||
           iterator_rhs != rhs.end() )
    {
      // the last bound (+∞) should be the same for both partition
      assert(iterator_lhs != lhs.end() && iterator_rhs != rhs.end() );

      const half_interval& next_lhs = std::get<0>(*iterator_lhs);
      const half_interval& next_rhs = std::get<0>(*iterator_rhs);

      const half_interval& next = min(next_lhs, next_rhs);
      const A * next_value = merge_elements(std::get<1>(*iterator_lhs), std::get<1>(*iterator_rhs));
      assert(next_value != nullptr);

      // merge previous and next if they point to the same value
      if (next_value == previous_value) {
        result.pop_back();
      }
      previous_value = next_value;

      result.emplace_back(next, next_value);

      if (next_lhs <= next) {
        iterator_lhs++;
      }
      if (next_rhs <= next) {
        iterator_rhs++;
      }
    }
  }

  template<class A>
  const A * lookup_partition(partition<A> const & boundaries, double value) {
    auto iterator = boundaries.begin();
    const A * result = std::get<1>(*iterator);
    while (!contains(std::get<0>(*iterator), value)) {
      iterator++;
      assert(iterator != boundaries.end());
      result = std::get<1>(*iterator);
    }
    return result;
  }

  template<class A>
  size_t hash_partition(partition<A>& boundaries, std::function<size_t (const A *)> hash_element) {
    //inspired by https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp

    size_t h1 = 0x3141592653589793; // seed
    const uint64_t c1 = 0x87c37b91114253d5;
    const uint64_t c2 = 0x4cf5ad432745937f;

    for(auto iterator = boundaries.begin(); iterator != boundaries.end(); iterator++) {
      size_t k1 = *(reinterpret_cast<const size_t*>(&(std::get<0>(std::get<0>(*iterator)))));
      switch ( std::get<1>(std::get<0>(*iterator)) ) {
        case Open:
          k1 ^= 0x239b961bab0e9789;
          break;
        default:
          k1 ^= 0x38b34ae5a1e38b93;
          break;
      }
      k1 *= c1;
      k1 = rotl64(k1,31);
      k1 *= c2;
      h1 ^= k1;
      h1 = rotl64(h1,27);
      h1 = h1*5+0x52dce729;

      h1 ^= hash_element(std::get<1>(*iterator));
    }

    h1 ^= boundaries.size();

    h1 ^= h1 >> 33;
    h1 *= 0xff51afd7ed558ccd;
    h1 ^= h1 >> 33;
    h1 *= 0xc4ceb9fe1a85ec53;
    h1 ^= h1 >> 33;

    return h1;
  }

  // Gets the RHS of the boundaries that are contained.
  template<class A>
  void interval_covers(std::list<const A*>& result, const partition<A>& boundaries, const interval& intv) {
    // Partitions are implicitly downwards closed towards -00, so the first element is finite.
    result.clear();
    // Create half_intervals based on our original for easier comparison.
    const half_interval intv_low = starts_after(intv);
    const half_interval intv_high = ends(intv);
    auto high = boundaries.begin();
    auto end  = boundaries.end();
    // First thing we do is explore the lower end.
    if (std::get<0>(intv_low) == -std::numeric_limits<double>::infinity() && std::get<0>(*(boundaries.begin())) <= intv_high) {
      result.push_back(std::get<1>(*(boundaries.begin())));
    }
    if (high != end) {
      // Use two iterators to compare consecutive elements.
      for (auto low = high++; high != end; low++, high++) {
        if (!(std::get<0>(*high) <= intv_high)) break;  // Reached the end!
        if (std::get<0>(*low) <= intv_low) continue;  // LHS of partition is too low!
        result.push_back(std::get<1>(*high));
      }
    }
  }

  template <class A>
  void interval_covered_by(std::list<const A*>& result, const partition <A>& boundaries, const interval& intv) {
    result.clear();
    // Create half intervals based on our original for easier comparison.
    const half_interval intv_low = starts_after(intv);
    const half_interval intv_high = ends(intv);
    // Two interators for consecutive item comparison.
    auto high = boundaries.begin();
    auto end  = boundaries.end();
    // We are always covered by the lower stuff from -00 to the first boundary.
    result.push_back(std::get<1>(*(boundaries.begin())));
    if (high != end) {
      for (auto low = high++; high != end; low++, high++) {
        if (!(std::get<0>(*low) <= intv_high)) break;
        if (std::get<0>(*high) <= intv_low) continue;
        result.push_back(std::get<1>(*high));
      }
    }
  }

  template<class A>
  std::ostream & print_partition(std::ostream & out, const partition<A>& boundaries, std::function<std::ostream & (std::ostream &, const A *)> print_element) {
    for(auto iterator = boundaries.begin(); iterator != boundaries.end(); iterator++) {
      print_element(out, std::get<1>(*iterator)) << " until " << std::get<0>(*iterator) << ", ";
    }
    return out;
  }

  template<class A>
  std::ostream & operator<<(std::ostream & out, const partition<A>& boundaries) {
    for(auto iterator = boundaries.begin(); iterator != boundaries.end(); iterator++) {
      out << *std::get<1>(*iterator) << " until " << std::get<0>(*iterator) << ", ";
    }
    return out;
  }
}
