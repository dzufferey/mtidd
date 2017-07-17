#pragma once

#include "partition.h"
//#include <iostream>

namespace mtidd
{

  template<class A>
  class partition_iterator: public std::iterator<std::forward_iterator_tag, const A*> {

    private:
    typedef typename std::list<std::tuple<half_interval, const A *>>::const_iterator boundaries_iterator;
    typedef std::function<bool (half_interval const & lower_bound, half_interval const & upper_bound, const A *)> filter_fun;

    filter_fun filter;
    half_interval lb;
    boundaries_iterator itr;
    boundaries_iterator last;

    void until_filter(){
      // find the first element that pass filter
      while(itr != last && !filter(lb, std::get<0>(*itr), std::get<1>(*itr))) {
        lb = complement(std::get<0>(*itr));
        ++itr;
      }
    }

    filter_fun trivial() const {
      return [](half_interval const &, half_interval const &, const A*) {
        return true;
      };
    }
   

    partition_iterator(partition<A> const & boundaries,
                       boundaries_iterator const& position):
        filter(trivial()),
        lb(lower_sentinel),
        itr(position),
        last(boundaries.cend()) {
    }

    public:

    partition_iterator(partition<A> const & boundaries):
        filter(trivial()),
        lb(lower_sentinel),
        itr(boundaries.cbegin()),
        last(boundaries.cend()) {
      until_filter();
    }
    
    partition_iterator(partition<A> const & boundaries, filter_fun filter_elements):
        filter(filter_elements),
        lb(lower_sentinel),
        itr(boundaries.cbegin()),
        last(boundaries.cend()) {
      until_filter();
    }

    partition_iterator<A>& operator++() {
      if (itr != last) {
        lb = complement(std::get<0>(*itr));
        ++itr;
        until_filter();
      }
      return *this;
    }

    partition_iterator<A> operator++(int) {
      partition_iterator<A> retval = *this;
      ++(*this);
      return retval;
    }
    bool operator==(partition_iterator<A> const & other) const {
      return itr == other.itr;
    }
    bool operator!=(partition_iterator<A> const & other) const {
      return !(*this == other);
    }
    const A* operator*() const {
      return std::get<1>(*itr);
    }

    static partition_iterator<A> end(partition<A> const & boundaries) {
      return partition_iterator<A>(boundaries, boundaries.cend());
    }

    // return the values that cover intv
    static partition_iterator<A> covers(partition<A> const & boundaries,  const interval& intv) {
      assert(!is_empty(intv));
      filter_fun  f = [&](half_interval const & a, half_interval const & b, const A*) {
        interval i = make_interval(a, b); 
        //bool res = overlap(i, intv);
        //std::cerr << "overlap(" << i << ", " << intv << ") = " << res << std::endl;
        return overlap(i, intv);
      };
      return partition_iterator<A>(boundaries, f);
    }
    
    // return the values fully covered by intv
    static partition_iterator<A> covered_by(partition<A> const & boundaries,  const interval& intv) {
      assert(!is_empty(intv));
      filter_fun  f = [&](half_interval const & a, half_interval const & b, const A*) {
        interval i = make_interval(a, b); 
        return mtidd::covers(intv, i);
      };
      return partition_iterator<A>(boundaries, f);
    }

  };


}
