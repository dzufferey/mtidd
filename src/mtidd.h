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
    V* variable; // we assume that the pointer equality is can be used to determine variable equality
    T* terminal;
    partition<idd<V, T, L>> part;
    // behind the scene
    IddManager* manager;
    size_t hash_value; // caching the hash for faster look-up

    // invariant:
    // variable = null ⇒ part = null && terminal ≠ null
    // variable ≠ null ⇒ part ≠ null && terminal = null

    void computeHash() {
      // hash variable
      // hash terminal
      // hash the partition
      // ...
    }
    
    idd<V, T, L> const & traverse(idd<V, T, L> const& rhs, bool lub) const {
      assert(manager == rhs.manager);
      if (is_terminal()) {
        if (rhs.is_terminal()) {
          if (lub) {
            T new_terminal = L.least_upper_bound( *terminal, *(rhs.terminal));
            *idd<V, T, L> result = new idd(manager, ??? ) //TODO get the address of new_terminal and not leak memory ...
            return manager.internalize(result);
          } else {
            T new_terminal = L.greatest_lower_bound( *terminal, *(rhs.terminal));
            *idd<V, T, L> result = new idd(manager, ??? ) //TODO get the address of new_terminal and not leak memory ...
            return manager.internalize(result);
          }
        } else {
          return rhs.traverse(this, lub);
        }
      } else {
        if (rhs.is_terminal()) {
          // map this.partition with rhs
          *idd<V, T, L> result = new idd(manager, variable, &L.bot);
          map_partition(result->partition, [](*idd<V, T, L> x) { return x->traverse(rhs, lub); } );
          result.computeHash();
          return manager.internalize(result);
        } else {
          int delta_v = manager.compare(*variable, *(that.variable));
          if (delta_v < 0) {
            // this is before: map this.partition with rhs
            *idd<V, T, L> result = new idd(manager, variable, &L.bot);
            map_partition(result->partition, [](*idd<V, T, L> x) { return x->traverse(rhs, lub); } );
            result.computeHash();
            return manager.internalize(result);
          } else if (delta_v > 0) {
            // rhs is before this
            return rhs.traverse(this, lub);
          } else {
            // same variable: merge the partitions
            *idd<V, T, L> result = new idd(manager, variable, &L.bot);
            merge_partition(result->partition, this.partition, rhs.partition, [](*idd<V, T, L> x, *idd<V, T, L> y) { return x->traverse(*y, lub); });
            result.computeHash();
            return manager.internalize(result);
          }
        }
      }
    }

  public:
    /*
    idd<V, T, L>& operator!() const {
      //XXX ¬ only make sense for the boolean case
    }
    */
    
    idd(IddManager* mngr, T* value): variable(nullptr), terminal(value), part(nullptr), manager(mngr) {
      computeHash();
    }

    idd(IddManager* mngr, V* var, const interval & i, const idd<V, T, L> & in, const idd<V, T, L> & out): variable(var), terminal(nullptr), part(new_partition(&out)), manager(mngr) {
      insert_partition(part, i, &in);
      computeHash();
    }

    // make sure the copy constructor is not called implicitly
    explicit idd(const idd<V, T, L>& that) {
      // XXX
    }

    bool is_terminal() {
      return variable == nullptr
    }

    idd<V, T, L> const & operator&&(const idd<V, T, L> & rhs) const {
      return traverse(rhs, true);
    }

    idd<V, T, L> const & operator||(const idd<V, T, L> & rhs) const {
      return traverse(rhs, false);
    }

    // only one step local, assume decendant are internalized in the manager
    bool operator==(idd<V, T, L> const& rhs) const {
      // only compare IDD from the same manager!
      assert(manager == rhs.manager);

      return hash_value != rhs.hash_value && (
               ( is_terminal() &&  rhs.is_terminal() && L.equal(*terminal == *(rhs.terminal)) ||
               (!is_terminal() && !rhs.is_terminal() && variable == rhs.variable && part = rhs.part)
             );
    }

    // assume the IDDs are internalized so equal means are the object
    bool compare_internalized(idd<V, T, L> const& rhs) {
      assert(manager == rhs.manager);
      return (&this) == (&rhs);
    }

    bool operator!=(idd<V, T, L> const& rhs) {
      assert(manager == rhs.manager);
      return (&this) != (&rhs);
    }

    bool operator<(idd<V, T, L> const& rhs) {
      // XXX
    }

    T lookup(std::map<V,double> const& point) const {
      // XXX
    }

    size_t hash() const {
      return hash_value;
    }
  }

  template< class V, // variable
            class T, // terminal
            class L = struct lattice<T> >
  struct idd_hash {
    size_t operator()(idd<V,T,L> * const s) const
    {
      return s->hash();
    }
  }
  
  template< class V, // variable
            class T, // terminal
            class L = struct lattice<T> >
  struct idd_equalTo {
    size_t operator()(idd<V,T,L> * const lhs, idd<V,T,L> * const rhs) const
    {
      return *lhs == *rhs;
    }
  }


  template< class V, // variable
            class T, // terminal
            class L = struct lattice<T> >
  class idd_manager
  {
  private:
    typedef unordered_set<idd<V,T,L>*, // keeps pointer around but use the hash and equality of the underlying object
                  idd_hash<V,T,L>,
                  idd_equalTo<V,T,L>> cache_t;
    cache_t cache;
    map<V, int> ordering_by_variable;
    vector<V> ordering_by_index; // XXX duplication of V in the vector or the map which one should prevail ?
  public:

    int compare(V const & v1, V const & v2) {
      return ordering_by_variable[v1] - ordering_by_variable[v2];
    }

    V* with_index(int n) {
      assert(n < number_of_variables() && n >= 0);
      return &ordering_by_index[n];
    }

    int number_of_variables() {
      return ordering_by_index.size();
    }

    idd<V, T, L> const & internalize(*idd<V, T, L> i) {
      cache_t::const_iterator found = cache.find(i);
      if (found == cache.end()) {
        cache.insert(i);
        return *i;
      } else {
        delete i;
        return *found;
      }
    }

    void releaseBut(idd<V, T, L> const & keep) {
      // TODO
      // traverse keep and get all the pointers
      // take the difference between the cache and the pointers to keep
      // free the difference
      // set the pointers to keep as the new cache
    }

    // constructs an IDD from a box (map v -> interval), a value, and a default value
    idd<V, T, L> const & idd(std::map<V,interval> const& box, T* inside_value, T* outside_value) {
      idd<V, T, L> const & out_part = internalize(new idd<V,T,L>(this, outside_value));
      idd<V, T, L> const & in_part = internalize(new idd<V,T,L>(this, outside_value));
      // start from the leaves and work our way up
      int n = number_of_variables();
      --n;
      while(n >= 0) {
        V& var = ordering_by_index[n];
        if (box.count(var) > 0) {
          const interval& i = box[var];
          if (is_empty(i)) {
            return out_part;
          } else {
            auto dd = new idd<V,T,L>(this, var, i, in_part, out_part);
            in_part = internalize(dd);
          }
        }
        --n;
      }
      return in_part;
    }

  }

} // end namespace
