#pragma once

#include <list>
#include <tuple>
#include <limits>
#include <iterator>

// TODO represent partitions with DLL of open/closed boundaries over double

using namespace std;

namespace mtidd
{

  typedef enum { Open, Closed } interval_boundary;

  interval_boundary complement(interval_boundary b);

  typedef tuple<double,interval_boundary> half_interval;

  // lhs is the right/end of an interval
  // rhs is the left/start of an interval
  bool ends_before(const half_interval& lhs, const half_interval& rhs);

  // lhs and rhs are the right/end of an interval
  bool operator<=(const half_interval& lhs, const half_interval& rhs);

  typedef tuple<double,interval_boundary,double,interval_boundary> interval;

  template<class A>
  class partition
  {
  private:
    list<tuple<half_interval, A*>> boundaries;
  public:
    // sentinel nodes
    static half_interval lower_sentinel = make_tuple<double, interval_boundary>(-numeric_limits<double>::infinity(), Open);
    static half_interval upper_sentinel = make_tuple<double, interval_boundary>( numeric_limits<double>::infinity(), Open);

    // alloc and dealloc
    partition(A* default_value): boundaries() {
      boundaries.push_back(make_tuple<half_interval, A*>(upper_sentinel, default_value)); //TODO emplace?
    }
    ~partition() {
    }

    //default copy constructor and assigment should be fine.

    // insert
    void insert(interval& const i, A* value) {
      auto new_start = make_tuple<double, interval_boundary>(get<0>(i), get<1>(i));
      auto iterator = boundaries.begin();
      // find where to start
      while ( ends_before(get<0>(iterator*), get<0>(i)) ) {
        ++iterator;
      }
      // insert new boundary if needed
      auto next_stop = get<0>(iterator*);
      bool overlap = get<0>(next_stop) > get<0>(new_start) || (get<1>(next_stop) == Closed && get<1>(new_start) == Closed);
      // overlap && old_value != new_value
      if ( overlap && get<1>(iterator*) != value ) {
         // insert new stop with value of old_stop
         auto complement_of_new = make_tuple<double, interval_boundary>(get<0>(new_start), complement(get<1>(new_start)));
         boundaries.insert(iterator, make_tuple<half_interval, A*>(complement_of_new, get<1>(iterator*))); //TODO emplace ?
      }
      ++iterator;
      // find where to stop
      while ( get<0>(iterator*) <= get<0>(i) && iterator != boundaries.end() ) {
        auto to_remove = iterator;
        ++iterator;
        boundaries.erase(to_remove);
      }
      // insert new boundary if needed
      if ( iterator == boundaries.end() || get<1>(iterator*) != value ) {
        auto new_stop = make_tuple<double, interval_boundary>(get<2>(i), get<3>(i));
        boundaries.insert(iterator, make_tuple<half_interval, A*>(new_stop, value)); //TODO emplace ?
      }
    }

    //TODO a method to merge to partition and combine the values ...

    // traverse: iterator<input_iterator_tag, tuple<interval,A*>> iterator();
    partition_iterator<A> iterator() {
      // XXX
    }
  };

  template<class A>
  class partition_iterator: public iterator<input_iterator_tag, tuple<interval, A*>>
  {
  private:
    list<half_interval, A*>::iterator it;
    half_interval previous;
  public:
    partition_iterator(list<half_interval, A*>::iterator& i) : it(i), previous(partition.lower_sentinel) {}
    partition_iterator(const partition_iterator<A>& x) : it(x.it), previous(x.previous) {}
    // XXX
    partition_iterator& operator++();
    partition_iterator operator++(int);
    bool operator==(const partition_iterator& rhs);
    bool operator!=(const partition_iterator& rhs);
    tuple<interval,A*>& operator*();
  };

}
