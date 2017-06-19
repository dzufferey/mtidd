#pragma once

#include <list>
#include <unordered_set>
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include "lattice.h"
#include "partition.h"
#include "internalizer.h"

using namespace std;


//TODO idd as nested class in manager to reduce the number of type arguments

namespace mtidd
{

  template< class V, // variable
            class T, // terminal
            class L = struct lattice<T> >
  class idd_manager;

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
      size_t k1 = (vi << 32) | ti;
      k1 *= c1;
      k1 = rotl64(k1,31);
      k1 *= c2;
      h1 ^= k1;
      h1 = rotl64(h1,27);
      h1 = h1*5+0x52dce729;
    
      function<size_t (const idd<V, T, L>*)> hasher = [&](const idd<V, T, L>* x) -> size_t {
        return x->hash();
      };

      h1 ^= hash_partition(part, hasher);

      h1 ^= h1 >> 33;
      h1 *= 0xff51afd7ed558ccd;
      h1 ^= h1 >> 33;
      h1 *= 0xc4ceb9fe1a85ec53;
      h1 ^= h1 >> 33;

      hash_value = h1;
    }
    
    idd<V, T, L> const & merge(idd<V, T, L> const& rhs, bool lub) const {
      assert(manager == rhs.manager);
      if (is_terminal()) {
        if (rhs.is_terminal()) {
          if (lub) {
            int new_terminal = manager->terminal_lub( terminal_index, rhs.terminal_index);
            idd<V, T, L>* result = new idd(manager, new_terminal);
            return manager->internalize(result);
          } else {
            int new_terminal = manager->terminal_glb( terminal_index, rhs.terminal_index);
            idd<V, T, L>* result = new idd(manager, new_terminal);
            return manager->internalize(result);
          }
        } else {
          return rhs.merge(*this, lub);
        }
      } else {
        if (rhs.is_terminal()) {
          // map this.partition with rhs
          idd<V, T, L>* result = new idd(manager, variable_index, manager->bottom());
          function<const idd<V, T, L>* (const idd<V, T, L>*)> mapper = [&](const idd<V, T, L>* x) {
            return &(x->merge(rhs, lub));
          };
          map_partition(result->part, part, mapper);
          result->computeHash();
          return manager->internalize(result);
        } else {
          int delta_v = variable_index - rhs.variable_index;
          if (delta_v < 0) {
            // this is before: map this.partition with rhs
            idd<V, T, L>* result = new idd(manager, variable_index, manager->bottom());
            function<const idd<V, T, L>* (const idd<V, T, L>*)> mapper = [&](const idd<V, T, L>* x) {
              return &(x->merge(rhs, lub));
            };
            map_partition(result->part, part, mapper);
            result->computeHash();
            return manager->internalize(result);
          } else if (delta_v > 0) {
            // rhs is before this
            return rhs.merge(*this, lub);
          } else {
            // same variable: merge the partitions
            idd<V, T, L>* result = new idd(manager, variable_index, manager->bottom());
            function<const idd<V, T, L>* (const idd<V, T, L>*, const idd<V, T, L>*)> merger = [&](const idd<V, T, L>* x, const idd<V, T, L>* y) {
              return &(x->merge(*y, lub));
            };
            merge_partition(result->part, part, rhs.part, merger);
            result->computeHash();
            return manager->internalize(result);
          }
        }
      }
    }

  public:
    
    idd(idd_manager<V,T,L>* mngr, int terminal_idx): variable_index(-1), terminal_index(terminal_idx), part(), manager(mngr) {
      computeHash();
    }

    idd(idd_manager<V,T,L>* mngr, int var_idx, const idd<V, T, L> & out): variable_index(var_idx), terminal_index(-1), part(new_partition(&out)), manager(mngr) {
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

    bool is_terminal() const {
      return variable_index < 0;
    }

    idd<V, T, L> const & operator&(const idd<V, T, L> & rhs) const {
      return merge(rhs, true);
    }

    idd<V, T, L> const & operator|(const idd<V, T, L> & rhs) const {
      return merge(rhs, false);
    }

    // only one step local, assume decendant are internalized in the manager
    bool compare_structural(idd<V, T, L> const& rhs) const {
      // only compare IDD from the same manager!
      assert(manager == rhs.manager);

      return hash_value == rhs.hash_value && (
               ( is_terminal() &&  rhs.is_terminal() && terminal_index == rhs.terminal_index) ||
               (!is_terminal() && !rhs.is_terminal() && variable_index == rhs.variable_index && part == rhs.part)
             );
    }

    // assume the IDDs are internalized so equal means are the object
    bool operator==(idd<V, T, L> const& rhs) const {
      assert(manager == rhs.manager);
      return this == (&rhs);
    }

    bool operator!=(idd<V, T, L> const& rhs) const {
      assert(manager == rhs.manager);
      return this != (&rhs);
    }

    bool operator<(idd<V, T, L> const& rhs) const {
      // XXX
      throw "TODO inclusion check";
    }

    const T& lookup(std::map<V,double> const& point) const {
      if (is_terminal()) {
        T const & ref = manager->terminal_at(terminal_index);
        return ref;
      } else {
        V const & var = manager->variable_at(variable_index);
        double value = point.at(var);
        idd<V, T, L> const * child = lookup_partition(part, value);
        return child->lookup(point);
      }
    }

    size_t hash() const {
      return hash_value;
    }

    void traverse(function<void (const idd<V, T, L> *)> apply_on_element) const {
      apply_on_element(this);
      if ( !is_terminal() ) {
        for(auto iterator = part.begin(); iterator != part.end(); iterator++) {
          get<1>(*iterator)->traverse(apply_on_element);
        }
      }
    }

  };
  
  template< class T, class L = class lattice<T> >
  struct lattice_hash {
    L lattice; // = L();
    size_t operator()(T const & t) const
    {
      return lattice.hash(t);
    }
  };

  template< class T, class L = struct lattice<T> >
  struct lattice_equalTo {
    L lattice; // = L();
    size_t operator()(T const & a, T const & b) const
    {
      return lattice.equal(a, b);
    }
  };

  template< class V, // variable
            class T, // terminal
            class L = struct lattice<T> >
  struct idd_hash {
    size_t operator()(idd<V,T,L> * const s) const
    {
      return s->hash();
    }
  };
  
  template< class V, // variable
            class T, // terminal
            class L = struct lattice<T> >
  struct idd_equalTo {
    size_t operator()(idd<V,T,L> * const lhs, idd<V,T,L> * const rhs) const
    {
      return lhs->compare_structural(*rhs);
    }
  };


  template< class V, // variable
            class T, // terminal
            class L >
  class idd_manager
  {
  private:
    L lattice;// = L();

    typedef unordered_set<idd<V,T,L>*, // keeps pointer around but use the hash and equality of the underlying object
                  idd_hash<V,T,L>,
                  idd_equalTo<V,T,L>> cache_t;
    cache_t cache;
    internalizer<V> variable_ordering; // trust the default hash and equal
    internalizer<T, lattice_hash<T,L>, lattice_equalTo<T,L>> terminals_store;


  public:

    //TODO change the ordering

    int compare(V const & v1, V const & v2) {
      return variable_ordering.compare(v1, v2);
    }

    int number_of_variables() {
      return variable_ordering.number_of_elements();
    }

    idd<V, T, L> const & internalize(idd<V, T, L>* i) {
      auto found = cache.find(i);
      if (found == cache.end()) {
        cache.insert(i);
        return *i;
      } else {
        delete i;
        return *(*found);
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
      T new_t = lattice.least_upper_bound(terminals_store.at(idx1), terminals_store.at(idx2));
      return internalize_terminal(new_t);
    }
    
    int terminal_glb(int idx1, int idx2) {
      T new_t = lattice.greatest_lower_bound(terminals_store.at(idx1), terminals_store.at(idx2));
      return internalize_terminal(new_t);
    }

    void release_except(idd<V, T, L> const & keep) {
      cache_t old_cache = cache;
      cache.clear();
      function<void (const idd<V, T, L> *)> traverser = [&](const idd<V, T, L>* x) { old_cache.erase(x); cache.insert(x); };
      keep.traverse(traverser);
      for (auto iterator = old_cache.begin(); iterator != old_cache.end(); iterator++) {
        delete(*iterator);
      }
    }

    idd<V, T, L> const & from_terminal(T const & value) {
      int idx = internalize_terminal(value);
      return internalize(new idd<V,T,L>(this, idx));
    }

    // constructs an IDD from a box (map v -> interval), a value, and a default value
    idd<V, T, L> const & from_box(std::map<V,interval> const& box, T const & inside_value, T const & outside_value) {
      int in_idx = internalize_terminal(inside_value);
      idd<V, T, L> const & in_part = internalize(new idd<V,T,L>(this, in_idx));
      int out_idx = internalize_terminal(outside_value);
      idd<V, T, L> const & out_part = internalize(new idd<V,T,L>(this, out_idx));
      // start from the leaves and work our way up
      idd<V, T, L> const * result = &in_part;
      int n = number_of_variables();
      --n;
      while(n >= 0) {
        V const & var = variable_ordering.at(n);
        if (box.count(var) > 0) {
          const interval & i = box.at(var);
          if (is_empty(i)) {
            return out_part;
          } else {
            auto dd = new idd<V,T,L>(this, n, i, *result, out_part);
            result = &internalize(dd);
          }
        }
        --n;
      }
      return *result;
    }

    //TODO not very efficient

    idd<V, T, L> const & top() {
      int idx = internalize_terminal(lattice.top());
      return internalize(new idd<V,T,L>(this, idx));
    }

    idd<V, T, L> const & bottom() {
      int idx = internalize_terminal(lattice.bottom());
      return internalize(new idd<V,T,L>(this, idx));
    }

  };

  template< class V > // variable
  class idd_manager<V, bool, lattice<bool>>
  {
  private:
    lattice<bool> lb;

    typedef unordered_set<idd<V,bool,lattice<bool>>*, // keeps pointer around but use the hash and equality of the underlying object
                  idd_hash<V,bool,lattice<bool>>,
                  idd_equalTo<V,bool,lattice<bool>>> cache_t;
    cache_t cache;
    internalizer<V> variable_ordering; // trust the default hash and equal

    int true_index = 1;
    int false_index = 0;

    bool true_ref = true;
    bool false_ref = false;

  public:

    //TODO change the ordering

    int compare(V const & v1, V const & v2) {
      return variable_ordering.compare(v1, v2);
    }

    int number_of_variables() {
      return variable_ordering.number_of_elements();
    }

    idd<V, bool, lattice<bool>> const & internalize(idd<V, bool, lattice<bool>>* i) {
      auto found = cache.find(i);
      if (found == cache.end()) {
        cache.insert(i);
        return *i;
      } else {
        delete i;
        return *(*found);
      }
    }

    int internalize_variable(V const & v) {
      return variable_ordering.internalize(v);
    }

    int internalize_terminal(bool const & t) {
      if (t) {
          return true_index;
      } else {
          return false_index;
      }
    }

    V const & variable_at(int index) {
      return variable_ordering.at(index);
    }

    bool const & terminal_at(int index) {
      if (index == true_index) {
        return true_ref;
      } else {
        return false_ref;
      }
    }

    int terminal_lub(int idx1, int idx2) {
      bool new_t = lb.least_upper_bound(terminal_at(idx1), terminal_at(idx2));
      return internalize_terminal(new_t);
    }
    
    int terminal_glb(int idx1, int idx2) {
      bool new_t = lb.greatest_lower_bound(terminal_at(idx1), terminal_at(idx2));
      return internalize_terminal(new_t);
    }

    void release_except(idd<V, bool, lattice<bool>> const & keep) {
      cache_t old_cache = cache;
      cache.clear();
      function<void (const idd<V, bool, lattice<bool>> *)> traverser = [&](const idd<V, bool, lattice<bool>>* x) { old_cache.erase(x); cache.insert(x); };
      keep.traverse(traverser);
      for (auto iterator = old_cache.begin(); iterator != old_cache.end(); iterator++) {
        delete(*iterator);
      }
    }

    idd<V, bool, lattice<bool>> const & from_terminal(bool const & value) {
      int idx = internalize_terminal(value);
      return internalize(new idd<V,bool,lattice<bool>>(this, idx));
    }

    // constructs an IDD from a box (map v -> interval), a value, and a default value
    idd<V, bool, lattice<bool>> const & from_box(std::map<V,interval> const& box, bool const & inside_value, bool const & outside_value) {
      int in_idx = internalize_terminal(inside_value);
      idd<V, bool, lattice<bool>> const & in_part = internalize(new idd<V,bool,lattice<bool>>(this, in_idx));
      int out_idx = internalize_terminal(outside_value);
      idd<V, bool, lattice<bool>> const & out_part = internalize(new idd<V,bool,lattice<bool>>(this, out_idx));
      // start from the leaves and work our way up
      idd<V, bool, lattice<bool>> const * result = &in_part;
      int n = number_of_variables();
      --n;
      while(n >= 0) {
        V const & var = variable_ordering.at(n);
        if (box.count(var) > 0) {
          const interval & i = box.at(var);
          if (is_empty(i)) {
            return out_part;
          } else {
            auto dd = new idd<V,bool,lattice<bool>>(this, n, i, *result, out_part);
            result = &internalize(dd);
          }
        }
        --n;
      }
      return *result;
    }

    //TODO not very efficient

    idd<V, bool, lattice<bool>> const & top() {
      return internalize(new idd<V,bool,lattice<bool>>(this, true_index));
    }

    idd<V, bool, lattice<bool>> const & bottom() {
      int idx = internalize_terminal(lb.bottom());
      return internalize(new idd<V,bool,lattice<bool>>(this, false_index));
    }

  };

} // end namespace
