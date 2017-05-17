#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

using namespace std;



namespace mtidd
{

  // takes, stores object, and give an int reference
  // the goal is to put all the object in a single place and not leak memory
  // implicitely this also gives an order on the stored elements
  template< class T, class Hash = hash<T>, class Equal = equal_to<T> >
  class internalizer
  {
  private:
    unordered_map<T, int, Hash, Equal> by_value;
    vector<V> by_index;
    // XXX duplication of V in the vector or the map which one should prevail ?
  public:

    int index(T const & v) {
      assert(by_value.count(v) > 0);
      return by_value[v];
    }

    T const & at(int i) {
      assert(i >= 0 && i < by_index.size());
      return by_index[i];
    }

    int compare(T const & v1, T const & v2) {
      assert(by_value.count(v1) > 0);
      assert(by_value.count(v2) > 0);
      return by_value[v1] - by_value[v2];
    }

    int number_of_elements() {
      assert(by_value.size() == by_index.size());
      return by_index.size();
    }

    int internalize(T const & v) {
      cache_t::const_iterator found = by_value.find(v);
      if (found == by_value.end()) {
        int i = by_index.size();
        by_index.emplace_back(T);
        by_value.emplace(T, i);
        return i;
      } else {
        return *found;
      }
    }

    //TODO method to clear some but not all
    //TODO method to permute the ordering

  }

} // end namespace
