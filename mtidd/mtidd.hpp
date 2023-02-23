#pragma once

#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "cache.hpp"
#include "internalizer.hpp"
#include "lattice.hpp"
#include "partition.hpp"
#include "partition_iterator.hpp"
#include "utils.hpp"

// TODO idd as nested class in manager to reduce the number of type arguments

// TODO alpha when the internalized orders change

// TODO operations as friend so we don't refer to this without the shared_ptr

// TODO bool, int specialization of the idd_manager, no need to internalize values
//  int true_index = 1;
//  int false_index = 0;

namespace mtidd {

template <typename V, // variable
          typename T, // terminal
          typename L = struct lattice<T>>
class idd_manager;

// to use idd* with unordered_set
template <typename V, // variable
          typename T, // terminal
          typename L = struct lattice<T>>
struct idd_hash;

// to use std::weak_ptr<idd> with unordered_set
template <typename V, // variable
          typename T, // terminal
          typename L = struct lattice<T>>
struct idd_hash_w;

// to use idd* with unordered_set
template <typename V, // variable
          typename T, // terminal
          typename L = struct lattice<T>>
struct idd_equalTo;

// to use std::weak_ptr<idd> with unordered_set
template <typename V, // variable
          typename T, // terminal
          typename L = struct lattice<T>>
struct idd_equalTo_w;

template <typename V, // variable
          typename T, // terminal
          typename L = struct lattice<T>>
class idd {

  public:
    typedef std::shared_ptr<const idd<V, T, L>> idd_t;

  private:
    typedef std::shared_ptr<idd<V, T, L>> idd_t_mutable;

    // node information
    int variable_index;
    int terminal_index;
    partition<const idd<V, T, L>> part;
    // behind the scene
    idd_manager<V, T, L> *manager;
    size_t hash_value; // caching the hash for faster look-up

    // invariant:
    // variable = null ⇒ part = null && terminal ≠ null
    // variable ≠ null ⇒ part ≠ null && terminal = null

    typedef std::unordered_set<idd_t, // for traversal only, not modification
                               idd_hash<V, T, L>, idd_equalTo<V, T, L>>
        idd_set;

    template <typename R, bool commutative = false>
    using operation_cache = cache<idd_t, R, commutative, idd_hash<V, T, L>>;

    typedef operation_cache<idd_t, true> operation_cache_idd;

    // TODO check https://github.com/Cyan4973/xxHash
    void computeHash() {
        // inspired by https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp

        size_t h1 = 0x3141592653589793; // seed
        const uint64_t c1 = 0x87c37b91114253d5;
        const uint64_t c2 = 0x4cf5ad432745937f;

        size_t vi = variable_index;
        size_t ti = terminal_index;
        size_t k1 = (vi << 32) | ti;
        k1 *= c1;
        k1 = rotl64(k1, 31);
        k1 *= c2;
        h1 ^= k1;
        h1 = rotl64(h1, 27);
        h1 = h1 * 5 + 0x52dce729;

        std::function<size_t(const idd<V, T, L> &)> hasher = [&](const idd<V, T, L> &x) -> size_t { return x.hash(); };

        h1 ^= hash_partition(part, hasher);

        h1 ^= h1 >> 33;
        h1 *= 0xff51afd7ed558ccd;
        h1 ^= h1 >> 33;
        h1 *= 0xc4ceb9fe1a85ec53;
        h1 ^= h1 >> 33;

        hash_value = h1;
    }

    friend idd_t check_trivial(idd_t_mutable result) {
        if (result->part.size() == 1) {
            return std::get<1>(*(result->part.begin()));
        } else {
            result->computeHash();
            return result->manager->internalize(result);
        }
    }

    friend idd_t combine1(idd_t lhs, idd_t rhs, std::function<T(T const &, T const &)> combine_elements,
                          operation_cache_idd &cache) {
        auto cached = cache.lookup(lhs, rhs);
        auto mngr = lhs->manager;
        if (cached) {
            return cached;
        } else if (lhs->is_terminal()) {
            if (rhs->is_terminal()) {
                T const &left = mngr->terminal_at(lhs->terminal_index);
                T const &right = mngr->terminal_at(rhs->terminal_index);
                T comb = combine_elements(left, right);
                int new_terminal = mngr->internalize_terminal(comb);
                idd_t result = std::make_shared<const idd<V, T, L>>(mngr, new_terminal);
                idd_t internalized = mngr->internalize(result);
                cache.insert(lhs, rhs, internalized);
                return internalized;
            } else {
                return combine1(rhs, lhs, combine_elements, cache);
            }
        } else {
            if (rhs->is_terminal()) {
                // map this.partition with rhs
                idd_t_mutable result = std::make_shared<idd<V, T, L>>(mngr, lhs->variable_index, mngr->bottom());
                std::function<idd_t(idd_t)> mapper = [&](idd_t x) { return combine1(x, rhs, combine_elements, cache); };
                map_partition<const idd<V, T, L>>(result->part, lhs->part, mapper);
                idd_t internalized = check_trivial(result);
                cache.insert(lhs, rhs, internalized);
                return internalized;
            } else {
                int delta_v = lhs->variable_index - rhs->variable_index;
                if (delta_v < 0) {
                    // this is before: map this.partition with rhs
                    idd_t_mutable result = std::make_shared<idd<V, T, L>>(mngr, lhs->variable_index, mngr->bottom());
                    std::function<idd_t(idd_t)> mapper = [&](idd_t x) {
                        return combine1(x, rhs, combine_elements, cache);
                    };
                    map_partition<const idd<V, T, L>>(result->part, lhs->part, mapper);
                    idd_t internalized = check_trivial(result);
                    cache.insert(lhs, rhs, internalized);
                    return internalized;
                } else if (delta_v > 0) {
                    // rhs is before this
                    return combine1(rhs, lhs, combine_elements, cache);
                } else {
                    // same variable: merge the partitions
                    idd_t_mutable result = std::make_shared<idd<V, T, L>>(mngr, lhs->variable_index, mngr->bottom());
                    std::function<idd_t(idd_t, idd_t)> merger = [&](idd_t x, idd_t y) {
                        return combine1(x, y, combine_elements, cache);
                    };
                    merge_partition<const idd<V, T, L>>(result->part, lhs->part, rhs->part, merger);
                    idd_t internalized = check_trivial(result);
                    cache.insert(lhs, rhs, internalized);
                    return internalized;
                }
            }
        }
    }

    friend void traverse1(idd_t lhs, std::function<void(idd_t)> apply_on_element, idd_set &cache) {
        if (cache.find(lhs) == cache.end()) {
            apply_on_element(lhs);
            cache.insert(lhs);
            if (!lhs->is_terminal()) {
                for (auto iterator = lhs->part.begin(); iterator != lhs->part.end(); iterator++) {
                    traverse1(std::get<1>(*iterator), apply_on_element, cache);
                }
            }
        }
    }

    friend void inf_terminal_cover1(idd_t lhs, std::map<V, interval> const &box, idd_set &cache,
                                    std::unordered_set<T> &terminals) {
        if (cache.find(lhs) == cache.end()) {
            cache.insert(lhs);
            if (!lhs->is_terminal()) {
                V const &var = lhs->manager->variable_at(lhs->variable_index);
                const interval var_intv = box.find(var)->second;
                auto end = partition_iterator<const idd<V, T, L>>::end(lhs->part);
                for (auto iterator = partition_iterator<const idd<V, T, L>>::covers(lhs->part, var_intv);
                     iterator != end; ++iterator) {
                    inf_terminal_cover1(*iterator, box, cache, terminals);
                }
            } else {
                terminals.insert(lhs->manager->terminal_at(lhs->terminal_index));
            }
        }
    }

    friend void partial_overlaps1(idd_t lhs, std::map<V, interval> &lhs_overlaps, std::map<V, interval> &rhs_overlaps,
                                  std::map<V, interval> const &box, idd_set &cache) {
        if (!(cache.find(lhs) == cache.end()))
            return;
        cache.insert(lhs);
        if (lhs->is_terminal())
            return;

        V const &var = lhs->manager->variable_at(lhs->variable_index);
        auto mb_intv = box.find(var);
        assert(mb_intv != box.end());
        interval cintv = mb_intv->second;
        half_interval curr_lb = std::make_tuple(std::get<0>(cintv), std::get<1>(cintv));
        half_interval curr_ub = std::make_tuple(std::get<2>(cintv), std::get<3>(cintv));

        auto first = lhs->part.begin();
        auto second = lhs->part.begin();
        second++;

        // The first scenario for the LHS is the lower sentinel.
        if (curr_lb <= std::get<0>(*first)) {
            lhs_overlaps[var] = make_interval(lower_sentinel, std::get<0>(*first));
        }

        for (; second != lhs->part.end(); first++, second++) {
            partial_overlaps1(std::get<1>(*first), lhs_overlaps, rhs_overlaps, box, cache);

            half_interval const &hintv1 = std::get<0>(*first);
            half_interval const &hintv2 = std::get<0>(*second);

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

    int idd_size1(idd_set &cache) const {
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

    friend lattice_compare compare1(idd_t lhs, idd_t rhs, operation_cache<lattice_compare> &cache) {
        if (lhs.get() == rhs.get()) {
            return lattice_compare::equivalent;
        } else if (cache.contains(lhs, rhs)) {
            return cache.lookup(lhs, rhs);
        } else {
            lattice_compare result = lattice_compare::equivalent;
            if (lhs->is_terminal()) {
                if (rhs->is_terminal()) {
                    T const &left = lhs->manager->terminal_at(lhs->terminal_index);
                    T const &right = rhs->manager->terminal_at(rhs->terminal_index);
                    result = L{}.compare(left, right);
                } else {
                    return flip(compare1(rhs, lhs, cache));
                }
            } else {
                if (rhs->is_terminal() || lhs->variable_index < rhs->variable_index) {
                    for (auto iterator = lhs->part.begin(); iterator != lhs->part.end(); iterator++) {
                        idd_t child = std::get<1>(*iterator);
                        result = result & compare1(child, rhs, cache);
                        if (result == lattice_compare::unordered) {
                            break;
                        }
                    }
                } else if (lhs->variable_index > rhs->variable_index) {
                    return flip(compare1(rhs, lhs, cache));
                } else {
                    auto iterator_lhs = lhs->part.begin();
                    auto iterator_rhs = rhs->part.begin();
                    while (result != lattice_compare::unordered &&
                           (iterator_lhs != lhs->part.end() || iterator_rhs != rhs->part.end())) {
                        // travers the partitions in parallel and recurse
                        // code copied and modified from merge_partition
                        // the last bound (+∞) should be the same for both
                        // partition
                        assert(iterator_lhs != lhs->part.end() && iterator_rhs != rhs->part.end());

                        idd_t child_lhs = std::get<1>(*iterator_lhs);
                        result = result & compare1(child_lhs, std::get<1>(*iterator_rhs), cache);

                        const half_interval &next_lhs = std::get<0>(*iterator_lhs);
                        const half_interval &next_rhs = std::get<0>(*iterator_rhs);
                        const half_interval &next = min(next_lhs, next_rhs);

                        if (next_lhs <= next) {
                            iterator_lhs++;
                        }
                        if (next_rhs <= next) {
                            iterator_rhs++;
                        }
                    }
                }
            }
            cache.insert(lhs, rhs, result);
            cache.insert(rhs, lhs, flip(result));
            return result;
        }
    }

  public:
    idd(idd_manager<V, T, L> *mngr, int terminal_idx)
        : variable_index(-1), terminal_index(terminal_idx), part(), manager(mngr) {
        computeHash();
    }

    idd(idd_manager<V, T, L> *mngr, int var_idx, idd_t out)
        : variable_index(var_idx), terminal_index(-1), part(new_partition(out)), manager(mngr) {
        computeHash();
    }

    idd(idd_manager<V, T, L> *mngr, int var_idx, const interval &i, idd_t in, idd_t out)
        : variable_index(var_idx), terminal_index(-1), part(new_partition(out)), manager(mngr) {
        insert_partition(part, i, in);
        computeHash();
    }

    // make sure the copy constructor is not called implicitly
    explicit idd(const idd<V, T, L> &that) { throw "The copy constructor of IDD should not be used!"; }

    bool is_terminal() const { return variable_index < 0; }

    /* Combines two idd, merging the terminal using the given function.
     * For instance, '&' and '|' just call this method with glb/lub.
     */
    friend idd_t combine(idd_t lhs, idd_t rhs, std::function<T(T const &, T const &)> combine_elements) {
        assert(lhs->manager == rhs->manager);
        operation_cache_idd cache(nullptr);
        return combine1(lhs, rhs, combine_elements, cache);
    }

    /* 'and' operator: combines two idd using the greatest lower bound */
    friend idd_t operator&(idd_t lhs, idd_t rhs) {
        std::function<T(T const &, T const &)> op = [&](T const &l, T const &r) {
            return L().greatest_lower_bound(l, r);
        };
        return combine(lhs, rhs, op);
    }

    /* 'or' operator: combines two idd using the least upper bound */
    friend idd_t operator|(idd_t lhs, idd_t rhs) {
        std::function<T(T const &, T const &)> op = [&](T const &l, T const &r) { return L().least_upper_bound(l, r); };
        return combine(lhs, rhs, op);
    }

    /* Test the structural equality between two idds.
     * This assumes that the children are alreadt internalized, therefore, the check is one for
     * the first/top-level node.
     */
    bool equals_structural(idd<V, T, L> const &rhs) const {
        // only compare IDD from the same manager!
        assert(manager == rhs.manager);

        return hash_value == rhs.hash_value &&
               ((is_terminal() && rhs.is_terminal() && terminal_index == rhs.terminal_index) ||
                (!is_terminal() && !rhs.is_terminal() && variable_index == rhs.variable_index && part == rhs.part));
    }

    bool equals_structural(idd_t rhs) const { return equals_structural(*rhs); }

    /* Testing equality.
     * Assume the IDDs are internalized so equality means their are the same objects.
     */
    bool operator==(idd<V, T, L> const &rhs) const {
        assert(manager == rhs.manager);
        return this == (&rhs);
    }

    friend bool operator==(idd_t lhs, idd_t rhs) {
        assert(lhs->manager == rhs->manager);
        return lhs.get() == rhs.get();
    }

    bool operator!=(idd<V, T, L> const &rhs) const {
        assert(manager == rhs.manager);
        return this != (&rhs);
    }

    friend bool operator!=(idd_t lhs, idd_t rhs) {
        assert(lhs->manager == rhs->manager);
        return lhs.get() != rhs.get();
    }

    /* Compare (partial-order) two idds. */
    friend lattice_compare compare(idd_t lhs, idd_t rhs) {
        assert(lhs->manager == rhs->manager);
        operation_cache<lattice_compare> cache(lattice_compare::unordered);
        return compare1(lhs, rhs, cache);
    }

    friend bool operator<(idd_t lhs, idd_t rhs) {
        lattice_compare c = compare(lhs, rhs);
        return c == lattice_compare::less;
    }

    friend bool operator<=(idd_t lhs, idd_t rhs) {
        lattice_compare c = compare(lhs, rhs);
        return c == lattice_compare::less || c == lattice_compare::equivalent;
    }

    friend bool operator>(idd_t lhs, idd_t rhs) {
        lattice_compare c = compare(lhs, rhs);
        return c == lattice_compare::greater;
    }

    friend bool operator>=(idd_t lhs, idd_t rhs) {
        lattice_compare c = compare(lhs, rhs);
        return c == lattice_compare::greater || c == lattice_compare::equivalent;
    }

    /* Given a value for each variable returns the corresponding terminal. */
    const T &lookup(std::map<V, double> const &point) const {
        if (is_terminal()) {
            T const &ref = manager->terminal_at(terminal_index);
            return ref;
        } else {
            V const &var = manager->variable_at(variable_index);
            double value = point.at(var);
            idd_t child = lookup_partition(part, value);
            return child->lookup(point);
        }
    }

    size_t hash() const { return hash_value; }

    /* Traverse recursively the idds nodes.
     * This method does not take sharing into account and can visit nodes multiple times.
     */
    friend void traverse_all(idd_t lhs, std::function<void(idd_t)> apply_on_element) {
        apply_on_element(lhs);
        if (!lhs->is_terminal()) {
            for (auto iterator = lhs->part.begin(); iterator != lhs->part.end(); iterator++) {
                traverse(std::get<1>(*iterator), apply_on_element);
            }
        }
    }

    /* Traverse recursively the idds nodes.
     * This method takes sharing into account and visit each node only once.
     */
    friend void traverse(idd_t lhs, std::function<void(idd_t)> apply_on_element) {
        idd_set cache;
        traverse1(lhs, apply_on_element, cache);
    }

    friend std::unordered_set<T> inf_terminal_cover(idd_t lhs, std::map<V, interval> const &box) {
        idd_set cache;
        std::unordered_set<T> terminals;
        inf_terminal_cover1(lhs, box, cache, terminals);
        return terminals;
    }

    friend void partial_overlaps(idd_t lhs, std::map<V, interval> &lhs_overlaps, std::map<V, interval> &rhs_overlaps,
                                 std::map<V, interval> const &box) {
        lhs_overlaps.clear();
        rhs_overlaps.clear();
        idd_set cache;
        partial_overlaps1(lhs, lhs_overlaps, rhs_overlaps, box, cache);
    }

    /* Returns the number of nodes in the idd. */
    int idd_size() {
        idd_set cache;
        return idd_size1(cache);
    }

    std::ostream &print(std::ostream &out, int indent = 0) const {
        for (int i = 0; i < indent; i++)
            out << " ";
        out << "idd(" << hash_value << ")";
        out << std::endl;
        for (int i = 0; i < indent; i++)
            out << " ";
        if (terminal_index >= 0) {
            out << "terminal: " << manager->terminal_at(terminal_index);
        } else {
            out << "variable: " << manager->variable_at(variable_index);
            for (auto iterator = part.begin(); iterator != part.end(); iterator++) {
                out << std::endl;
                for (int i = 0; i < indent; i++)
                    out << " ";
                out << "until " << std::get<0>(*iterator) << std::endl;
                std::get<1>(*iterator)->print(out, indent + 2);
            }
        }
        return out;
    }

}; // idd

template <typename V, typename T, typename L> struct idd_hash {
    size_t operator()(std::shared_ptr<idd<V, T, L> const> s) const { return s->hash(); }
};

template <typename V, typename T, typename L> struct idd_hash_w {
    size_t operator()(std::weak_ptr<idd<V, T, L> const> w) const {
        if (auto observe = w.lock()) {
            return observe->hash();
        } else {
            return 0;
        }
    }
};

template <typename V, typename T, typename L> struct idd_equalTo {
    bool operator()(std::shared_ptr<idd<V, T, L> const> lhs, std::shared_ptr<idd<V, T, L> const> rhs) const {
        return lhs->equals_structural(*rhs);
    }
};

template <typename V, typename T, typename L> struct idd_equalTo_w {
    bool operator()(std::weak_ptr<idd<V, T, L> const> lhs, std::weak_ptr<idd<V, T, L> const> rhs) const {
        if (auto l = lhs.lock()) {
            if (auto r = rhs.lock()) {
                return l->equals_structural(*r);
            }
        }
        return false;
    }
};

template <typename V, // variable
          typename T, // terminal
          typename L>
class idd_manager {
  public:
    typedef std::shared_ptr<const idd<V, T, L>> idd_t;

  private:
    typedef std::unordered_set<std::weak_ptr<const idd<V, T, L>>, // probably a bad idea!
                               idd_hash_w<V, T, L>, idd_equalTo_w<V, T, L>>
        cache_t;
    cache_t cache;
    internalizer<V> variable_ordering; // trust the default hash and equal
    internalizer<T, lattice_hash<T, L>, lattice_equalTo<T, L>> terminals_store;

    // caching top and bottom as they are often called
    idd_t _top = std::shared_ptr<const idd<V, T, L>>(nullptr);
    idd_t _bottom = std::shared_ptr<const idd<V, T, L>>(nullptr);

  public:
    int compare(V const &v1, V const &v2) { return variable_ordering.compare(v1, v2); }

    int number_of_variables() { return variable_ordering.number_of_elements(); }

    idd_t internalize(idd_t i) {
        auto found = cache.find(i);
        if (found == cache.end()) {
            cache.insert(i);
            return i;
        } else {
            if (auto shared = found->lock()) {
                return shared;
            } else {
                cache.erase(found);
                cache.insert(i);
                return i;
            }
        }
    }

    int internalize_variable(V const &v) { return variable_ordering.internalize(v); }

    int internalize_terminal(T const &t) { return terminals_store.internalize(t); }

    V variable_at(int index) { return variable_ordering.at(index); }

    T terminal_at(int index) { return terminals_store.at(index); }

    T terminal_lub(T const &l, T const &r) { return L().least_upper_bound(l, r); }

    T terminal_glb(T const &l, T const &r) { return L().greatest_lower_bound(l, r); }

    /* Constructs an IDD from a default value. */
    idd_t from_terminal(T const &value) {
        int idx = internalize_terminal(value);
        return internalize(std::make_shared<idd<V, T, L> const>(this, idx));
    }

    /* Constructs an IDD from a box (map v -> interval), a value, and a default value. */
    idd_t from_box(std::map<V, interval> const &box, T const &inside_value, T const &outside_value) {
        int in_idx = internalize_terminal(inside_value);
        idd_t in_part = internalize(std::make_shared<idd<V, T, L> const>(this, in_idx));
        int out_idx = internalize_terminal(outside_value);
        idd_t out_part = internalize(std::make_shared<idd<V, T, L> const>(this, out_idx));
        // start from the leaves and work our way up
        idd_t result = in_part;
        int n = number_of_variables();
        --n;
        while (n >= 0) {
            V const &var = variable_ordering.at(n);
            if (box.count(var) > 0) {
                const interval &i = box.at(var);
                if (is_empty(i)) {
                    return out_part;
                } else {
                    auto dd = std::make_shared<idd<V, T, L> const>(this, n, i, result, out_part);
                    result = internalize(dd);
                }
            }
            --n;
        }
        return result;
    }

    idd_t top() {
        if (_top.get() == nullptr) {
            _top = from_terminal(L().top());
        }
        return _top;
    }

    idd_t bottom() {
        if (_bottom.get() == nullptr) {
            _bottom = from_terminal(L().bottom());
        }
        return _bottom;
    }

    // TODO when done copy in the bool specialization
    std::map<idd<V, T, L> const &, idd<V, T, L> const &> reorder(std::map<int, int> const &var_idx_bijection) {
        int n = number_of_variables();
        std::map<int, int> inverse_bijection;
        // checks that we are indeed dealing with a bijection and that the indices are in
        // the [0,#var) range
        for (int i = 0; i < n; i++) {
            assert(var_idx_bijection.count(i) == 1);
            int j = var_idx_bijection.at(i);
            assert(j >= 0 && j < n);
            assert(inverse_bijection.count(j) == 0);
            inverse_bijection[j] = i;
        }
        // permute on the ordering
        variable_ordering.permute(var_idx_bijection);
        // TODO bubble sort on the idd
        int idx[n]; // keep track of the permutations
        for (int i = 0; i < n; i++) {
            idx[i] = inverse_bijection[i];
        }
        int end_index = n - 1;
        while (0 < end_index) {
            end_index = 0;
            int new_end_index = 0;
            for (int i = 1; i < end_index; i++) {
                if (idx[i - 1] > idx[i]) {
                    new_end_index = i;
                    int tmp = idx[i - 1];
                    idx[i - 1] = idx[i];
                    idx[i] = tmp;
                    // TODO swap i + 1 in the idd
                    // TODO mapping for the old idd to the new or can we do it
                    // in-place?
                    throw "TODO swap";
                }
            }
            end_index = new_end_index;
        }
        throw "TODO inplace?";
    }

    size_t size() const { return cache.size(); }

}; // idd_manager

template <typename V, // variable
          typename T, // terminal
          typename L = struct lattice<T>>
struct idd_pair_hash {
    size_t
    operator()(std::tuple<std::shared_ptr<idd<V, T, L> const>, std::shared_ptr<idd<V, T, L> const>> const &s) const {
        return combine_two_hashes(std::get<0>(s)->hash(), std::get<1>(s)->hash());
    }
};

// printing functions

template <typename V, // variable
          typename T, // terminal
          typename L>
std::ostream &operator<<(std::ostream &out, const idd<V, T, L> &dd) {
    return dd.print(out);
}

} // namespace mtidd
