#pragma once

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>

#include "internalizer.h"
#include "lattice.h"
#include "partition.h"
#include "partition_iterator.h"
#include "utils.h"
#include "cache.h"

//TODO idd as nested class in manager to reduce the number of type arguments

namespace mtidd
{

  template< typename V, // variable
            typename T, // terminal
            typename L = struct lattice<T> >
  class idd_manager;

  // to use idd* with unordered_set
  template< typename V, // variable
            typename T, // terminal
            typename L = struct lattice<T> >
  struct idd_hash;

  // to use idd* with unordered_set
  template< typename V, // variable
            typename T, // terminal
            typename L = struct lattice<T> >
  struct idd_equalTo;

  template< typename V, // variable
            typename T, // terminal
            typename L = struct lattice<T> >
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

    typedef std::unordered_set<idd<V,T,L> const *,
                               idd_hash<V,T,L>,
                               idd_equalTo<V,T,L>> idd_set;

    template< typename R, bool commutative = false>
    using operation_cache = cache<idd<V,T,L> const *, R, commutative, idd_hash<V,T,L>>;

    typedef operation_cache<idd<V,T,L> const *, true> operation_cache_idd;

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

      std::function<size_t (const idd<V, T, L>*)> hasher = [&](const idd<V, T, L>* x) -> size_t {
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

    idd<V, T, L> const & check_trivial(idd<V, T, L> * result) const {
      if (result->part.size() == 1) {
        idd<V, T, L>* result_old = result;
        idd<V, T, L> const & new_result = *std::get<1>(*(result->part.begin()));
        delete(result_old);
        return new_result;
      } else {
        result->computeHash();
        return manager->internalize(result);
      }
    }

    idd<V, T, L> const & combine1(idd<V, T, L> const& rhs,
                                  std::function<T (T const &, T const &)> combine_elements,
                                  operation_cache_idd & cache) const {
      auto cached = cache.lookup(this, &rhs);
      if (cached != nullptr) {
        return *cached;
      } else if (is_terminal()) {
        if (rhs.is_terminal()) {
          idd<V, T, L>* result;
          T const & left = manager->terminal_at(terminal_index);
          T const & right = manager->terminal_at(rhs.terminal_index);
          int new_terminal = manager->internalize_terminal( combine_elements(left, right) );
          result = new idd(manager, new_terminal);
          idd<V, T, L> const& internalized = manager->internalize(result);
          cache.insert(this, &rhs, &internalized);
          return internalized;
        } else {
          return rhs.combine1(*this, combine_elements, cache);
        }
      } else {
        if (rhs.is_terminal()) {
          // map this.partition with rhs
          idd<V, T, L>* result = new idd(manager, variable_index, manager->bottom());
          std::function<const idd<V, T, L>* (const idd<V, T, L>*)> mapper = [&](const idd<V, T, L>* x) {
            return &(x->combine1(rhs, combine_elements, cache));
          };
          map_partition(result->part, part, mapper);
          idd<V, T, L> const& internalized = check_trivial(result);
          cache.insert(this, &rhs, &internalized);
          return internalized;
        } else {
          int delta_v = variable_index - rhs.variable_index;
          if (delta_v < 0) {
            // this is before: map this.partition with rhs
            idd<V, T, L>* result = new idd(manager, variable_index, manager->bottom());
            std::function<const idd<V, T, L>* (const idd<V, T, L>*)> mapper = [&](const idd<V, T, L>* x) {
              return &(x->combine1(rhs, combine_elements, cache));
            };
            map_partition(result->part, part, mapper);
            idd<V, T, L> const& internalized = check_trivial(result);
            cache.insert(this, &rhs, &internalized);
            return internalized;
          } else if (delta_v > 0) {
            // rhs is before this
            return rhs.combine1(*this, combine_elements, cache);
          } else {
            // same variable: merge the partitions
            idd<V, T, L>* result = new idd(manager, variable_index, manager->bottom());
            std::function<const idd<V, T, L>* (const idd<V, T, L>*, const idd<V, T, L>*)> merger = [&](const idd<V, T, L>* x, const idd<V, T, L>* y) {
              return &(x->combine1(*y, combine_elements, cache));
            };
            merge_partition(result->part, part, rhs.part, merger);
            idd<V, T, L> const& internalized = check_trivial(result);
            cache.insert(this, &rhs, &internalized);
            return internalized;
          }
        }
      }
    }

    void traverse1(std::function<void (const idd<V, T, L> *)> apply_on_element, idd_set & cache) const {
      if (cache.find(this) == cache.end()) {
        apply_on_element(this);
        cache.insert(this);
        if ( !is_terminal() ) {
          for(auto iterator = part.begin(); iterator != part.end(); iterator++) {
            std::get<1>(*iterator)->traverse1(apply_on_element, cache);
          }
        }
      }
    }

    void inf_terminal_cover1(std::map<V,interval> const & box,
                             idd_set & cache,
                             std::unordered_set<T> & terminals) const {
      if (cache.find(this) == cache.end()) {
        cache.insert(this);
        if (!is_terminal()) {
          V const & var = manager->variable_at(variable_index);
          const interval var_intv = box.find(var)->second;
          auto end = partition_iterator<idd<V,T,L>>::end(part);
          for (auto iterator = partition_iterator<idd<V,T,L>>::covers(part,var_intv); iterator != end; ++iterator) {
            (*iterator)->inf_terminal_cover1(box, cache, terminals);
          }
        } else {
          terminals.insert(manager->terminal_at(terminal_index));
        }
      }
    }

    void partial_overlaps1(std::map<V,interval> & lhs_overlaps,
                           std::map<V,interval> & rhs_overlaps,
                           std::map<V,interval> const & box,
                           idd_set & cache) const {
      if (!(cache.find(this) == cache.end())) return;
      cache.insert(this);
      if (is_terminal()) return;

      V const & var = manager->variable_at(variable_index);
      auto mb_intv = box.find(var);
      assert(mb_intv != box.end());
      interval cintv = mb_intv->second;
      half_interval curr_lb = std::make_tuple(std::get<0>(cintv), std::get<1>(cintv));
      half_interval curr_ub = std::make_tuple(std::get<2>(cintv), std::get<3>(cintv));

      auto first = part.begin();
      auto second = part.begin();
      second++;

      // The first scenario for the LHS is the lower sentinel.
      if (curr_lb <= std::get<0>(*first)) {
        lhs_overlaps[var] = make_interval(lower_sentinel, std::get<0>(*first));
      }

      for (; second != part.end(); first++, second++) {
        std::get<1>(*first)->partial_overlaps1(lhs_overlaps, rhs_overlaps, box, cache);

        half_interval const & hintv1 = std::get<0>(*first);
        half_interval const & hintv2 = std::get<0>(*second);

        // This qualifies as a LHS partial cover.
        if (hintv1 <= curr_lb && curr_lb <= hintv2) {
          auto mb_lhs_intv = lhs_overlaps.find(var);
          // In the cache?
          if (mb_lhs_intv != lhs_overlaps.end()) {
            // We are more RIGHT than the previous.
            if (ends(mb_lhs_intv->second) < hintv2) {
              lhs_overlaps[var] = make_interval(hintv1, hintv2);
            }
          } else {
            lhs_overlaps[var] = make_interval(hintv1, hintv2);
          }
        }

        // This qualifies as a RHS partial cover.
        if (hintv1 <= curr_ub && curr_ub <= hintv2) {
          auto mb_rhs_intv = rhs_overlaps.find(var);
          // In the cache?
          if (mb_rhs_intv != rhs_overlaps.end()) {
            // We are more LEFT than the previous.
            if (starts_after(mb_rhs_intv->second) < hintv1) {
              rhs_overlaps[var] = make_interval(hintv1, hintv2);
            }
          } else {
            rhs_overlaps[var] = make_interval(hintv1, hintv2);
          }
        }
      }
    }

    int idd_size1(idd_set & cache) const {
      if (cache.find(this) == cache.end()) {
        cache.insert(this);
        if (!is_terminal()) {
          int sum = 0;
          for (auto iterator = part.begin(); iterator != part.end(); iterator++) {
            sum += std::get<1>(*iterator)->idd_size1(cache);
          }
          return sum;
        } else {
          return 1;
        }
      } else {
        return 0;
      }
    }

    lattice_compare compare1(idd<V, T, L> const & rhs, operation_cache<lattice_compare> & cache) const {
      if (*this == rhs) {
        return Equal;
      } else if (cache.contains(this, &rhs)) {
         return cache.lookup(this, &rhs);
      } else {
        lattice_compare result = Equal;
        if (is_terminal()) {
          if (rhs.is_terminal()) {
            T const & left = manager->terminal_at(terminal_index);
            T const & right = manager->terminal_at(rhs.terminal_index);
            result = L{}.compare(left, right);
          } else {
            return flip(rhs.compare1(*this, cache));
          }
        } else {
          if (rhs.is_terminal() || variable_index < rhs.variable_index) {
            for (auto iterator = part.begin(); iterator != part.end(); iterator++) {
              result = result && std::get<1>(*iterator)->compare1(rhs, cache);
              if (result == Different) {
                break;
              }
            }
          } else if (variable_index > rhs.variable_index) {
            return flip(rhs.compare1(*this, cache));
          } else {
            auto iterator_lhs = part.begin();
            auto iterator_rhs = rhs.part.begin();
            while (result != Different && (
                     iterator_lhs != part.end() ||
                     iterator_rhs != rhs.part.end()
                   ) )
            {
              // travers the partitions in parallel and recurse
              // code copied and modified from merge_partition
              // the last bound (+∞) should be the same for both partition
              assert(iterator_lhs != part.end() && iterator_rhs != rhs.part.end() );

              result = result && std::get<1>(*iterator_lhs)->compare1(*std::get<1>(*iterator_rhs), cache);

              const half_interval& next_lhs = std::get<0>(*iterator_lhs);
              const half_interval& next_rhs = std::get<0>(*iterator_rhs);
              const half_interval& next = min(next_lhs, next_rhs);

              if (next_lhs <= next) {
                iterator_lhs++;
              }
              if (next_rhs <= next) {
                iterator_rhs++;
              }
            }
          }
        }
        cache.insert(this, &rhs, result);
        cache.insert(&rhs, this, flip(result));
        return result;
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

    idd<V, T, L> const & combine(const idd<V, T, L> & rhs, std::function<T (T const &, T const &)> combine_elements) const {
      assert(manager == rhs.manager);
      operation_cache_idd cache(nullptr);
      return combine1(rhs, combine_elements, cache);
    }

    idd<V, T, L> const & operator&(const idd<V, T, L> & rhs) const {
      std::function<T (T const &, T const &)> op = [&](T const & l, T const &r) {
        return manager->terminal_glb(l, r);
      };
      return combine(rhs, op);
    }

    idd<V, T, L> const & operator|(const idd<V, T, L> & rhs) const {
      std::function<T (T const &, T const &)> op = [&](T const & l, T const &r) {
        return manager->terminal_lub(l, r);
      };
      return combine(rhs, op);
    }

    // only one step local, assume decendant are internalized in the manager
    bool equals_structural(idd<V, T, L> const& rhs) const {
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

    lattice_compare compare(idd<V, T, L> const & rhs) const {
      assert(manager == rhs.manager);
      operation_cache<lattice_compare> cache(Different);
      return compare1(rhs, cache);
    }

    bool operator<(idd<V, T, L> const& rhs) const {
      lattice_compare c = compare(rhs);
      return c == Smaller;
    }

    bool operator<=(idd<V, T, L> const& rhs) const {
      lattice_compare c = compare(rhs);
      return c == Smaller || c == Equal;
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

    void traverse_all(std::function<void (const idd<V, T, L> *)> apply_on_element) const {
      apply_on_element(this);
      if ( !is_terminal() ) {
        for(auto iterator = part.begin(); iterator != part.end(); iterator++) {
          std::get<1>(*iterator)->traverse(apply_on_element);
        }
      }
    }

    // take sharing into account and visit nodes just once
    void traverse(std::function<void (const idd<V, T, L> *)> apply_on_element) const {
      idd_set cache;
      traverse1(apply_on_element, cache);
    }

    std::unordered_set<T> inf_terminal_cover(std::map<V,interval> const& box) const {
      idd_set cache;
      std::unordered_set<T> terminals;
      inf_terminal_cover1(box, cache, terminals);
      return terminals;
    }

    void partial_overlaps(std::map<V,interval> & lhs_overlaps,
                          std::map<V,interval> & rhs_overlaps,
                          std::map<V,interval> const & box) const {
      lhs_overlaps.clear();
      rhs_overlaps.clear();
      idd_set cache;
      partial_overlaps1(lhs_overlaps, rhs_overlaps, box, cache);
    }

    int idd_size() {
      idd_set cache;
      return idd_size1(cache);
    }

    std::ostream & print(std::ostream & out, int indent = 0) const {
      for (int i = 0; i < indent; i++) out << " ";
      out << "idd(" << hash_value << ")";
      out << std::endl;
      for (int i = 0; i < indent; i++) out << " ";
      if (terminal_index >= 0) {
        out << "terminal: " << manager->terminal_at(terminal_index);
      } else {
        out << "variable: " << manager->variable_at(variable_index);
        for(auto iterator = part.begin(); iterator != part.end(); iterator++) {
          out << std::endl;
          for (int i = 0; i < indent; i++) out << " ";
          out << "until " << std::get<0>(*iterator) << std::endl;
          std::get<1>(*iterator)->print(out, indent + 2);
        }
      }
      return out;
    }

  };

  template< typename V, typename T, typename L >
  struct idd_hash {
    size_t operator()(idd<V,T,L> const * s) const
    {
      return s->hash();
    }
  };

  template< typename V, typename T, typename L >
  struct idd_equalTo {
    size_t operator()(idd<V,T,L> const * lhs, idd<V,T,L> const * rhs) const
    {
      return lhs->equals_structural(*rhs);
    }
  };


  template< typename V, // variable
            typename T, // terminal
            typename L >
  class idd_manager
  {
  private:
    L lattice;// = L();

    typedef std::unordered_set<const idd<V,T,L>*, // keeps pointer around but use the hash and equality of the underlying object
                  idd_hash<V,T,L>,
                  idd_equalTo<V,T,L>> cache_t;
    cache_t cache;
    internalizer<V> variable_ordering; // trust the default hash and equal
    internalizer<T, lattice_hash<T,L>, lattice_equalTo<T,L>> terminals_store;

    //caching top and bottom as they are often called
    idd<V, T, L> const * _top = nullptr;
    idd<V, T, L> const * _bottom = nullptr;


  public:

    ~idd_manager() {
      for (auto iterator = cache.begin(); iterator != cache.end(); iterator++) {
        delete(*iterator);
      }
    }

    //TODO change the ordering

    L const & get_lattice() { return lattice; }

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

    T terminal_lub(T const & l, T const & r) {
      return lattice.least_upper_bound(l, r);
    }

    T terminal_glb(T const & l, T const & r) {
      return lattice.greatest_lower_bound(l, r);
    }

    void release_except(idd<V, T, L> const & keep) {
      cache_t old_cache = cache;
      cache.clear();
      std::function<void (const idd<V, T, L> *)> traverser = [&](const idd<V, T, L>* x) { old_cache.erase(x); cache.insert(x); };
      keep.traverse(traverser);
      for (auto iterator = old_cache.begin(); iterator != old_cache.end(); iterator++) {
        if (*iterator == _top) {
          _top = nullptr;
        }
        if (*iterator == _bottom) {
          _bottom = nullptr;
        }
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

    idd<V, T, L> const & top() {
      if (_top == nullptr) {
        _top = &from_terminal(lattice.top());
      }
      return *_top;
    }

    idd<V, T, L> const & bottom() {
      if (_bottom == nullptr) {
        _bottom = &from_terminal(lattice.bottom());
      }
      return *_bottom;
    }

  }; // idd

  template< typename V > // variable
  class idd_manager<V, bool, lattice<bool>>
  {
  private:
    lattice<bool> lb;

    typedef std::unordered_set<const idd<V,bool,lattice<bool>>*, // keeps pointer around but use the hash and equality of the underlying object
                  idd_hash<V,bool,lattice<bool>>,
                  idd_equalTo<V,bool,lattice<bool>>> cache_t;
    cache_t cache;
    internalizer<V> variable_ordering; // trust the default hash and equal

    int true_index = 1;
    int false_index = 0;

    bool true_ref = true;
    bool false_ref = false;

    //caching top and bottom as they are often called
    idd<V, bool, lattice<bool>> const * _top = nullptr;
    idd<V, bool, lattice<bool>> const * _bottom = nullptr;

  public:

    ~idd_manager() {
      for (auto iterator = cache.begin(); iterator != cache.end(); iterator++) {
        delete(*iterator);
      }
    }

    //TODO change the ordering

    lattice<bool> const & get_lattice() { return lb; }

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
      std::function<void (const idd<V, bool, lattice<bool>> *)> traverser = [&](const idd<V, bool, lattice<bool>>* x) { old_cache.erase(x); cache.insert(x); };
      keep.traverse(traverser);
      for (auto iterator = old_cache.begin(); iterator != old_cache.end(); iterator++) {
        if (*iterator == _top) {
          _top = nullptr;
        }
        if (*iterator == _bottom) {
          _bottom = nullptr;
        }
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

    idd<V, bool, lattice<bool>> const & top() {
      if (_top == nullptr) {
        _top = &internalize(new idd<V,bool,lattice<bool>>(this, true_index));
      }
      return *_top;
    }

    idd<V, bool, lattice<bool>> const & bottom() {
      if (_bottom == nullptr) {
        _bottom = &internalize(new idd<V,bool,lattice<bool>>(this, false_index));
      }
      return *_bottom;
    }

  };

  template< typename V, // variable
            typename T, // terminal
            typename L = struct lattice<T> >
  struct idd_pair_hash {
    size_t operator()(std::tuple<idd<V,T,L> const *, idd<V,T,L> const * > const & s) const
    {
      return combine_two_hashes( std::get<0>(s)->hash(),  std::get<1>(s)->hash());
    }
  };


  // printing functions

  template< typename V, // variable
            typename T, // terminal
            typename L >
  std::ostream & operator<<(std::ostream & out, const idd<V,T,L>& dd) {
    return dd.print(out);
  }

} // end namespace
