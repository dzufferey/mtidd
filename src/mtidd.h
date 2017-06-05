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


//TODO idd as nested class in manager to reduce the number of type arguments

namespace mtidd
{

  template< class V, // variable
            class T, // terminal
            class L = struct lattice<T> >
  class idd
  {
  private:
    // node information
    int variable_index;
    int terminal_index;
    partition<idd<V, T, L>> part;
    // behind the scene
    idd_manager<V,T,L>* manager;
    size_t hash_value; // caching the hash for faster look-up

    // invariant:
    // variable = null ⇒ part = null && terminal ≠ null
    // variable ≠ null ⇒ part ≠ null && terminal = null

    void computeHash() {
      //inspired by https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp

      size_t h1 = 0x3141592653589793; // seed
      const uint64_t c1 = 0x87c37b91114253d5;
      const uint64_t c2 = 0x4cf5ad432745937f;

      size_t vi = variable_index;
      size_t ti = terminal_index;
      size_t k1 ^= (vi << 32) | ti;
      k1 *= c1;
      k1 = rotl64(k1,31);
      k1 *= c2;
      h1 ^= k1;
      h1 = rotl64(h1,27);
      h1 = h1*5+0x52dce729;

      h1 ^= hash_partition(part, [](*idd<V, T, L> x) { return x->hash(); });

      h1 ^= h1 >> 33;
      h1 *= 0xff51afd7ed558ccd;
      h1 ^= h1 >> 33;
      h1 *= 0xc4ceb9fe1a85ec53;
      h1 ^= h1 >> 33;

      return h1;
    }
    
    idd<V, T, L> const & merge(idd<V, T, L> const& rhs, bool lub) const {
      assert(manager == rhs.manager);
      if (is_terminal()) {
        if (rhs.is_terminal()) {
          if (lub) {
            int new_terminal = manager->terminal_lub( terminal_index, rhs.terminal_index);
            *idd<V, T, L> result = new idd(manager, new_terminal)
            return manager.internalize(result);
          } else {
            int new_terminal = manager->terminal_glb( terminal_index, rhs.terminal_index);
            *idd<V, T, L> result = new idd(manager, new_terminal)
            return manager.internalize(result);
          }
        } else {
          return rhs.merge(this, lub);
        }
      } else {
        if (rhs.is_terminal()) {
          // map this.partition with rhs
          *idd<V, T, L> result = new idd(manager, variable_index, &L.bot);
          map_partition(result->part, part, [](*idd<V, T, L> x) { return x->merge(rhs, lub); } );
          result.computeHash();
          return manager->internalize(result);
        } else {
          int delta_v = variable_index - that.variable_index;
          if (delta_v < 0) {
            // this is before: map this.partition with rhs
            *idd<V, T, L> result = new idd(manager, variable_index, &L.bot);
            map_partition(result->part, part, [](*idd<V, T, L> x) { return x->merge(rhs, lub); } );
            result.computeHash();
            return manager->internalize(result);
          } else if (delta_v > 0) {
            // rhs is before this
            return rhs.merge(this, lub);
          } else {
            // same variable: merge the partitions
            *idd<V, T, L> result = new idd(manager, variable_index, &L.bot);
            merge_partition(result->part, part, rhs.part, [](*idd<V, T, L> x, *idd<V, T, L> y) { return x->merge(*y, lub); });
            result.computeHash();
            return manager->internalize(result);
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
    
    idd(idd_manager<V,T,L>* mngr, T const & value): variable_index(-1), terminal_index(mngr->internalize_terminal(value)), part(nullptr), manager(mngr) {
      computeHash();
    }
    
    idd(idd_manager<V,T,L>* mngr, int terminal_idx): variable_index(-1), terminal_index(terminal_idx), part(nullptr), manager(mngr) {
      computeHash();
    }
    
    idd(idd_manager<V,T,L>* mngr, int var_idx, const interval & i, const idd<V, T, L> & in, const idd<V, T, L> & out): variable_index(var_idx), terminal_index(-1), part(new_partition(&out)), manager(mngr) {
      insert_partition(part, i, &in);
      computeHash();
    }

    //TODO alpha when the internalized orders change

    // make sure the copy constructor is not called implicitly
    explicit idd(const idd<V, T, L>& that) {
      throw "The copy constructor of IDD should not be used!";
    }

    bool is_terminal() {
      return variable_index < 0;
    }

    idd<V, T, L> const & operator&&(const idd<V, T, L> & rhs) const {
      return merge(rhs, true);
    }

    idd<V, T, L> const & operator||(const idd<V, T, L> & rhs) const {
      return merge(rhs, false);
    }

    // only one step local, assume decendant are internalized in the manager
    bool operator==(idd<V, T, L> const& rhs) const {
      // only compare IDD from the same manager!
      assert(manager == rhs.manager);

      return hash_value != rhs.hash_value && (
               ( is_terminal() &&  rhs.is_terminal() && terminal_index == rhs.terminal_index) ||
               (!is_terminal() && !rhs.is_terminal() && variable_index == rhs.variable_index && part == rhs.part)
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
      throw "TODO inclusion check";
    }

    const T& lookup(std::map<V,double> const& point) const {
      if (is_terminal()) {
        return manager->terminal_at(terminal_index);
      } else {
        return lookup_partition(part, point(*variable))->lookup(point);
      }
    }

    size_t hash() const {
      return hash_value;
    }

    void traverse(void (*apply_on_element)(idd<V, T, L> const&)) const {
      apply_on_element(this);
      if ( !is_terminal() ) {
        for(auto iterator = part.begin(); iterator != part.end(); iterator++) {
          iterator->get<1>()->traverse(apply_on_element);
        }
      }
    }

  }
  
  template< class T, class L = struct lattice<T> >
  struct latice_hash {
    size_t operator()(T const & t) const
    {
      return L.hash(t);
    }
  }

  template< class T, class L = struct lattice<T> >
  struct latice_equalTo {
    size_t operator()(T const & a, T const & b) const
    {
      return L.equal(a, b);
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
    internalizer<V> variable_ordering; // thrust the default hash and equal
    internalizer<T, latice_hash<T,L>, latice_equalTo<T,L>> terminals_store;
  public:

    //TODO change the ordering

    int compare(V const & v1, V const & v2) {
      return variable_ordering.compare(v1, v2);
    }

    int number_of_variables() {
      return variable_ordering.number_of_elements();
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

    int internalize_variable(V const & v) {
      return variable_ordering.internalize(v);
    }

    int internalize_terminal(T const & t) {
      return terminals_store.internalize(t);
    }

    V const & variable_at(int index) {
      return variable_ordering.at(index);
    }

    T const & terminal_at(int index) {
      return terminals_store.at(index);
    }

    int terminal_lub(int idx1, int idx2) {
      T new_t = L.least_upper_bound(terminals_store.at(idx1), terminals_store.at(idx2));
      return internalize_terminal(new_t);
    }
    
    T const & terminal_glb(int idx1, int idx2) {
      T new_t = L.greatest_lower_bound(terminals_store.at(idx1), terminals_store.at(idx2));
      return internalize_terminal(new_t);
    }

    void release_except(idd<V, T, L> const & keep) {
      cache_t old_cache = cache;
      cache.clear();
      keep.traverse([](*idd<V, T, L> x) { old_cache.erase(x); cache.insert(x); });
      for (auto iterator = old_cache.begin(); iterator != old_cache.end(); iterator++) {
        delete(*iterator);
      }
    }

    // constructs an IDD from a box (map v -> interval), a value, and a default value
    idd<V, T, L> const & idd(std::map<V,interval> const& box, T* inside_value, T* outside_value) {
      idd<V, T, L> const & out_part = internalize(new idd<V,T,L>(this, outside_value));
      idd<V, T, L> const & in_part = internalize(new idd<V,T,L>(this, outside_value));
      // start from the leaves and work our way up
      int n = number_of_variables();
      --n;
      while(n >= 0) {
        V& var = variable_ordering.at[n];
        if (box.count(var) > 0) {
          const interval& i = box[var];
          if (is_empty(i)) {
            return out_part;
          } else {
            auto dd = new idd<V,T,L>(this, n, i, in_part, out_part);
            in_part = internalize(dd);
          }
        }
        --n;
      }
      return in_part;
    }

  }

} // end namespace
