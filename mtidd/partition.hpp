#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <tuple>

#include "interval.hpp"
#include "utils.hpp"

// represent partitions with DLL of open/closed boundaries over double

namespace mtidd {

// a partition is just an alias for a list of boundaries
template <class A> using partition = std::list<std::tuple<half_interval, std::shared_ptr<const A>>>;

template <class A> partition<A> new_partition(std::shared_ptr<const A> default_value) {
    partition<A> boundaries;
    boundaries.emplace_back(upper_sentinel, default_value);
    return boundaries;
}

// insert an interval into a partition
template <class A> void insert_partition(partition<A> &boundaries, interval const &i, std::shared_ptr<const A> value) {
    // cerr << "inserting " << i << " in " << boundaries << endl;
    assert(!is_empty(i));
    half_interval new_start = std::make_tuple(std::get<0>(i), std::get<1>(i));
    auto iterator = boundaries.begin();
    // find where to start
    bool gap = true; // we only need to insert a new boundary when there is a gap with the
                     // previous one
    while (ends_before(std::get<0>(*iterator), new_start)) {
        gap = std::get<0>(std::get<0>(*iterator)) < std::get<0>(new_start);
        ++iterator;
    }
    // insert new boundary if needed
    auto next_stop = std::get<0>(*iterator);
    // cerr << "inserting at/before " << next_stop << endl;
    bool overlap = gap && (std::get<0>(next_stop) > std::get<0>(new_start) ||
                           (std::get<1>(next_stop) == Closed && std::get<1>(new_start) == Open));
    // overlap && old_value != new_value
    if (overlap && std::get<1>(*iterator) != value) {
        // insert new stop with value of old_stop
        half_interval complement_of_new = std::make_tuple(std::get<0>(new_start), complement(std::get<1>(new_start)));
        boundaries.emplace(iterator, complement_of_new, std::get<1>(*iterator));
    }
    // find where to stop
    half_interval new_stop = std::make_tuple(std::get<2>(i), std::get<3>(i));
    while (std::get<0>(*iterator) <= new_stop && iterator != boundaries.end()) {
        auto to_remove = iterator;
        ++iterator;
        boundaries.erase(to_remove);
    }
    // cerr << "stoping at/before " << std::get<0>(*iterator) << endl;
    //  insert new boundary if needed
    if (iterator == boundaries.end() || std::get<1>(*iterator) != value) {
        boundaries.emplace(iterator, new_stop, value);
    }
}

template <class A>
void map_partition(partition<A> &result, partition<A> const &arg,
                   std::function<std::shared_ptr<const A>(std::shared_ptr<const A>)> map_elements) {
    std::shared_ptr<const A> previous_value = nullptr;
    result.clear();
    auto iterator = arg.begin();
    while (iterator != arg.end()) {

        const half_interval &next = std::get<0>(*iterator);
        std::shared_ptr<const A> next_value = map_elements(std::get<1>(*iterator));
        assert(next_value.get() != nullptr);

        // merge previous and next if they point to the same value
        if (previous_value && next_value == previous_value) {
            result.pop_back();
        }
        previous_value = next_value;

        result.emplace_back(next, next_value);
        iterator++;
    }
}

template <class A>
void filter_partition(std::list<interval> &result, partition<A> const &arg,
                      std::function<bool(const A &)> filter_elements) {
    result.clear();
    auto iterator = arg.begin();
    half_interval lhs = lower_sentinel;
    half_interval rhs = std::get<0>(*iterator);
    while (iterator != arg.end()) {
        if (filter_elements(*std::get<1>(*iterator))) {
            interval intv = std::make_tuple(std::get<0>(lhs), std::get<1>(lhs), std::get<0>(rhs), std::get<1>(rhs));
            result.push_back(intv);
        }
        iterator++;
        lhs = std::make_tuple(std::get<0>(rhs), complement(std::get<1>(rhs)));
        rhs = std::get<0>(*iterator);
    }
}

// when calling the method, the accumulator contains the initial value.
// when returning, the accumulator contains the result
template <class A, class B>
void foldl_partition(B &accumulator, partition<A> const &arg, std::function<B(const B, const A &)> combine) {
    auto iterator = arg.begin();
    while (iterator != arg.end()) {
        accumulator = combine(accumulator, *std::get<1>(*iterator));
        iterator++;
    }
}

// a method to merge to partition and combine the values
// the result is stored into `result`
template <class A>
void merge_partition(
    partition<A> &result, partition<A> const &lhs, partition<A> const &rhs,
    std::function<std::shared_ptr<const A>(std::shared_ptr<const A>, std::shared_ptr<const A>)> merge_elements) {

    std::shared_ptr<const A> previous_value = nullptr;
    result.clear();

    auto iterator_lhs = lhs.begin();
    auto iterator_rhs = rhs.begin();

    while (iterator_lhs != lhs.end() || iterator_rhs != rhs.end()) {
        // the last bound (+∞) should be the same for both partition
        assert(iterator_lhs != lhs.end() && iterator_rhs != rhs.end());

        const half_interval &next_lhs = std::get<0>(*iterator_lhs);
        const half_interval &next_rhs = std::get<0>(*iterator_rhs);

        const half_interval &next = min(next_lhs, next_rhs);
        std::shared_ptr<const A> next_value = merge_elements(std::get<1>(*iterator_lhs), std::get<1>(*iterator_rhs));
        assert(next_value); // check for valid pointer

        // merge previous and next if they point to the same value
        if (previous_value && next_value == previous_value) {
            result.pop_back();
        }
        previous_value = next_value;

        result.emplace_back(next, next_value);

        if (next_lhs <= next) {
            iterator_lhs++;
        }
        if (next_rhs <= next) {
            iterator_rhs++;
        }
    }
}

template <class A> std::shared_ptr<const A> lookup_partition(partition<A> const &boundaries, double value) {
    auto iterator = boundaries.begin();
    std::shared_ptr<const A> result = std::get<1>(*iterator);
    while (!contains(std::get<0>(*iterator), value)) {
        iterator++;
        assert(iterator != boundaries.end());
        result = std::get<1>(*iterator);
    }
    return result;
}

template <class A> size_t hash_partition(partition<A> &boundaries, std::function<size_t(const A &)> hash_element) {
    // inspired by https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp

    size_t h1 = 0x3141592653589793; // seed
    const uint64_t c1 = 0x87c37b91114253d5;
    const uint64_t c2 = 0x4cf5ad432745937f;

    for (auto iterator = boundaries.begin(); iterator != boundaries.end(); iterator++) {
        size_t k1 = *(reinterpret_cast<const size_t *>(&(std::get<0>(std::get<0>(*iterator)))));
        switch (std::get<1>(std::get<0>(*iterator))) {
        case Open:
            k1 ^= 0x239b961bab0e9789;
            break;
        default:
            k1 ^= 0x38b34ae5a1e38b93;
            break;
        }
        k1 *= c1;
        k1 = rotl64(k1, 31);
        k1 *= c2;
        h1 ^= k1;
        h1 = rotl64(h1, 27);
        h1 = h1 * 5 + 0x52dce729;

        h1 ^= hash_element(*std::get<1>(*iterator));
    }

    h1 ^= boundaries.size();

    h1 ^= h1 >> 33;
    h1 *= 0xff51afd7ed558ccd;
    h1 ^= h1 >> 33;
    h1 *= 0xc4ceb9fe1a85ec53;
    h1 ^= h1 >> 33;

    return h1;
}

template <class A>
std::ostream &print_partition(std::ostream &out, const partition<A> &boundaries,
                              std::function<std::ostream &(std::ostream &, const A *)> print_element) {
    for (auto iterator = boundaries.begin(); iterator != boundaries.end(); iterator++) {
        print_element(out, *std::get<1>(*iterator)) << " until " << std::get<0>(*iterator) << ", ";
    }
    return out;
}

template <class A> std::ostream &operator<<(std::ostream &out, const partition<A> &boundaries) {
    for (auto iterator = boundaries.begin(); iterator != boundaries.end(); iterator++) {
        out << *std::get<1>(*iterator) << " until " << std::get<0>(*iterator) << ", ";
    }
    return out;
}
} // namespace mtidd
