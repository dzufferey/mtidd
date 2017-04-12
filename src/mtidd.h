#pragma once

#include <list>
#include <unordered_set>
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include "lattice.h"
#include "partition.h"

using namespace std;

namespace mtidd
{

  template< class V, // variable
            class T, // terminal
            class L = struct lattice<T> >
  class idd
  {
  private:
    // node infomration
    V* variable;
    bool is_terminal;
    T* terminal;
    partition<idd<V, T, L>> p;
    // behind the scene
    IddManager* manager;
    size_t hash_value; // caching the hash for faster look-up

    void computeHash() {
      // ...
    }

  public:
    //XXX ¬ only make sense for the boolean case
    /*
    idd<V, T, L>& operator!() const {
      // ...
    }
    */

    idd<V, T, L>& operator&&(idd<V, T, L> const& rhs) const {
      //TODO could be compute the hash first, do a lookup, and otherwise create the node
      // ...
    }

    idd<V, T, L>& operator||(idd<V, T, L> const& rhs) const {
      // ...
    }

    bool structually_equal(idd<V, T, L> const& rhs) {
      if (hash_value != ths.hash_value) {
        return false;
      } else {
        //TODO iterator and stuff
        // ...
      }
    }

    bool operator==(idd<V, T, L> const& rhs) {
      return (&this) == (&rhs);
    }

    bool operator!=(idd<V, T, L> const& rhs) {
      return (&this) != (&rhs);
    }

    bool operator<(idd<V, T, L> const& rhs) {
      // ...
    }

    T lookup(std::map<V,double> const& point) const {
      // ...
    }

    size_t hash() const {
      return hash_value;
    }
  }

  template< class V, // variable
            class T, // terminal
            class L = struct lattice<T> >
  class idd_manager
  {
  private:
    unordered_set<idd> cache;
    map<V, int> ordering_by_variable;
    vector<V> ordering_by_index;
  public:
    idd<V, T, L>& create(V const & variable, T value = L.bottom);
    idd<V, T, L>& create(V const & variable, T left_value, double bound, bool closed, T right_value);
  }

} // end namespace

// custom parial specialization of std::hash
namespace std
{
  template<V, T, L> struct hash<mtidd<V,T,L>>
  {
    typedef mtidd<V,T,L> argument_type;
    typedef size_t result_type;
    result_type operator()(argument_type const& s) const
    {
      return s.hash();
    }
  };
}
