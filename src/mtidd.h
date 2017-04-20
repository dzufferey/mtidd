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
    // node information
    bool is_terminal;
    V* variable; // we assume that the pointer equality is can be used to determine variable equality
    T* terminal;
    partition<idd<V, T, L>> part;
    // behind the scene
    IddManager* manager;
    size_t hash_value; // caching the hash for faster look-up

    // invariant:
    //  is_terminal ⇒ variable = null && part = null && terminal ≠ null
    // ¬is_terminal ⇒ variable ≠ null && part ≠ null && terminal = null

    void computeHash() {
      // hash is_terminal
      // hash variable
      // hash terminal
      // hash the partition
      // ...
    }

  public:
    /*
    idd<V, T, L>& operator!() const {
      //XXX ¬ only make sense for the boolean case
    }
    */

    // TODO access the variable ordering in the manager?

    // make sure the copy constructor is not called implicitly
    explicit idd(const idd<V, T, L>& that) {
      // XXX
    }

    idd<V, T, L>& operator&&(idd<V, T, L> const& rhs) const {
      //TODO could be compute the hash first, do a lookup, and otherwise create the node
      // ...
    }

    idd<V, T, L>& operator||(idd<V, T, L> const& rhs) const {
      // ...
    }

    // only one step local, assume decendant are internalized in the manager
    bool structually_equal(idd<V, T, L> const& rhs) {
      // only compare IDD from the same manager!
      assert(manager == rhs.manager);

      return hash_value != rhs.hash_value && (
               ( is_terminal &&  rhs.is_terminal && L.equal(*terminal == *(rhs.terminal)) ||
               (!is_terminal && !rhs.is_terminal && variable == rhs.variable && part = rhs.part)
             );
    }

    // assume the IDDs are internalized so equal means are the object
    bool operator==(idd<V, T, L> const& rhs) {
      assert(manager == rhs.manager);
      return (&this) == (&rhs);
    }

    bool operator!=(idd<V, T, L> const& rhs) {
      assert(manager == rhs.manager);
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
    int compare(V const & v1, V const & v2) {
      return ordering_by_variable[v1] - ordering_by_variable[v2];
    }
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
