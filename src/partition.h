#pragma once

#include <list>
#include <tuple>
#include <iterator>
#include "interval.h"

// TODO represent partitions with DLL of open/closed boundaries over double

using namespace std;

namespace mtidd
{

  ///////////////
  // partition //
  ///////////////

  // a partition is just an alias for a list of boundaries
  template<class A>
  using partition = list<tuple<half_interval, A*>>;

  template<class A>
  partition<A> new_partition(A* default_value) {
    partition<A> boundaries;
    boundaries.emplace_back(upper_sentinel, default_value);
    return boundaries;
  }

  // insert an interval into a partition
  template<class A>
  void insert(partition<A>& boundaries, interval const& i, A* value) {
    half_interval new_start = make_tuple(get<0>(i), get<1>(i));
    auto iterator = boundaries.begin();
    // find where to start
    while ( ends_before(get<0>(*iterator), new_start) ) {
      ++iterator;
    }
    // insert new boundary if needed
    auto next_stop = get<0>(*iterator);
    bool overlap = get<0>(next_stop) > get<0>(new_start) || (get<1>(next_stop) == Closed && get<1>(new_start) == Closed);
    // overlap && old_value != new_value
    if ( overlap && get<1>(*iterator) != value ) {
       // insert new stop with value of old_stop
       half_interval complement_of_new = make_tuple(get<0>(new_start), complement(get<1>(new_start)));
       boundaries.emplace(iterator, complement_of_new, get<1>(*iterator));
    }
    ++iterator;
    // find where to stop
    half_interval new_stop = make_tuple(get<2>(i), get<3>(i));
    while ( get<0>(*iterator) <= new_stop && iterator != boundaries.end() ) {
      auto to_remove = iterator;
      ++iterator;
      boundaries.erase(to_remove);
    }
    // insert new boundary if needed
    if ( iterator == boundaries.end() || get<1>(*iterator) != value ) {
      boundaries.insert(iterator, new_stop, value);
    }
  }

  //a method to merge to partition and combine the values
  //the result is stored into `result`
  template<class A>
  void merge(partition<A>& result, partition<A> const& lhs, partition<A> const& rhs, A* (*merge_elements)(A*,  A*)) {

    A* previous_value = nullptr;
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
      A* next_value = merge_elements(get<1>(*iterator_lhs), get<1>(*iterator_lhs));
      assert(next_value != nullptr);

      // merge previous and next if they point to the same value
      if (next_value == previous_value) {
        result.pop_back();
      }
      previous_value = next_value;

      result.emplace_back(next, next_value);

      if (next == next_lhs) {
        iterator_lhs++;
      }
      if (next == next_rhs) {
        iterator_rhs++;
      }
    }
  }

  template<class A>
  A* lookup(partition<A>& boundaries, double value) {
    A* result = nullptr;
    auto iterator = boundaries.begin();
    do {
      result = get<1>(*iterator);
      iterator++;
    } while (iterator != boundaries.end() && contains(get<0>(*iterator), value));
    return result;
  }

}
