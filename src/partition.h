#pragma once

#include <list>
#include <tuple>
#include <iterator>
#include <functional>
#include <iostream>
#include <assert.h>
#include "interval.h"
#include "utils.h"

// represent partitions with DLL of open/closed boundaries over double

using namespace std;

namespace mtidd
{

  // a partition is just an alias for a list of boundaries
  template<class A>
  using partition = list<tuple<half_interval, const A *>>;

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
    half_interval new_start = make_tuple(get<0>(i), get<1>(i));
    auto iterator = boundaries.begin();
    // find where to start
    bool gap = true; // we only need to insert a new boundary when there is a gap with the previous one
    while ( ends_before(get<0>(*iterator), new_start) ) {
      gap = get<0>(get<0>(*iterator)) < get<0>(new_start);
      ++iterator;
    }
    // insert new boundary if needed
    auto next_stop = get<0>(*iterator);
    //cerr << "inserting at/before " << next_stop << endl;
    bool overlap = gap && (get<0>(next_stop) > get<0>(new_start) || (get<1>(next_stop) == Closed && get<1>(new_start) == Open));
    // overlap && old_value != new_value
    if ( overlap && get<1>(*iterator) != value ) {
       // insert new stop with value of old_stop
       half_interval complement_of_new = make_tuple(get<0>(new_start), complement(get<1>(new_start)));
       boundaries.emplace(iterator, complement_of_new, get<1>(*iterator));
    }
    // find where to stop
    half_interval new_stop = make_tuple(get<2>(i), get<3>(i));
    while ( get<0>(*iterator) <= new_stop && iterator != boundaries.end() ) {
      auto to_remove = iterator;
      ++iterator;
      boundaries.erase(to_remove);
    }
    //cerr << "stoping at/before " << get<0>(*iterator) << endl;
    // insert new boundary if needed
    if ( iterator == boundaries.end() || get<1>(*iterator) != value ) {
      boundaries.emplace(iterator, new_stop, value);
    }
  }
  
  template<class A>
  void map_partition(partition<A>& result, partition<A> const& arg, function<const A* (const A *)> map_elements) {
    const A * previous_value = nullptr;
    result.clear();
    auto iterator = arg.begin();
    while (iterator != arg.end()){

      const half_interval& next = get<0>(*iterator);
      const A * next_value = map_elements(get<1>(*iterator));
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

  //a method to merge to partition and combine the values
  //the result is stored into `result`
  template<class A>
  void merge_partition(partition<A>& result, partition<A> const& lhs, partition<A> const& rhs, function<const A* (const A *, const A *)> merge_elements) {

    const A * previous_value = nullptr;
    result.clear();

    auto iterator_lhs = lhs.begin();
    auto iterator_rhs = rhs.begin();

    while (iterator_lhs != lhs.end() ||
           iterator_rhs != rhs.end() )
    {
      // the last bound (+∞) should be the same for both partition
      assert(iterator_lhs != lhs.end() && iterator_rhs != rhs.end() );

      const half_interval& next_lhs = get<0>(*iterator_lhs);
      const half_interval& next_rhs = get<0>(*iterator_rhs);

      const half_interval& next = min(next_lhs, next_rhs);
      const A * next_value = merge_elements(get<1>(*iterator_lhs), get<1>(*iterator_rhs));
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
    const A * result = get<1>(*iterator);
    while (!contains(get<0>(*iterator), value)) {
      iterator++;
      assert(iterator != boundaries.end());
      result = get<1>(*iterator);
    }
    return result;
  }

  template<class A>
  size_t hash_partition(partition<A>& boundaries, function<size_t (const A *)> hash_element) {
    //inspired by https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp

    size_t h1 = 0x3141592653589793; // seed
    const uint64_t c1 = 0x87c37b91114253d5;
    const uint64_t c2 = 0x4cf5ad432745937f;

    for(auto iterator = boundaries.begin(); iterator != boundaries.end(); iterator++) {
      size_t k1 = *(reinterpret_cast<const size_t*>(&(get<0>(get<0>(*iterator)))));
      switch ( get<1>(get<0>(*iterator)) ) {
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

      h1 ^= hash_element(get<1>(*iterator));
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
  std::list<const A *> interval_contents(const partition<A>& boundaries,
                                         const interval& intv) {
    // Partitions are implicitly downwards closed towards -00, so the first element is finite.
    std::list<const A *> elements;
    // Create half_intervals based on our original for easier comparison.
    const half_interval lower_bound = starts_after(intv);
    const half_interval upper_bound = ends(intv);
    // Use two iterators to compare consecutive elements.
    auto next = boundaries.begin();
    auto end  = boundaries.end();
    if (next != end) {
      for (auto curr = next++; next != end; curr++, next++) {
        if (!(get<0>(*next) <= upper_bound)) break;  // Reached the end!
        if (get<0>(*curr) <= lower_bound) continue;  // LHS of partition is too low!
        elements.push_back(get<1>(*next));
      }
    }
    return elements;
  }

  template<class A>
  ostream & print_partition(ostream & out, const partition<A>& boundaries, function<ostream & (ostream &, const A *)> print_element) {
    for(auto iterator = boundaries.begin(); iterator != boundaries.end(); iterator++) {
      print_element(out, get<1>(*iterator)) << " until " << get<0>(*iterator) << ", ";
    }
    return out;
  }

  template<class A>
  ostream & operator<<(ostream & out, const partition<A>& boundaries) {
    for(auto iterator = boundaries.begin(); iterator != boundaries.end(); iterator++) {
      out << *get<1>(*iterator) << " until " << get<0>(*iterator) << ", ";
    }
    return out;
  }
}
