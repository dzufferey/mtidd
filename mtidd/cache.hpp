#pragma once

#include <unordered_map>

#include "utils.hpp"

namespace mtidd {

// to cache the result of binary operations
template <typename A, // arguments
          typename R, // result
          bool commutative = false, typename Hash = std::hash<A>>
class cache {
  protected:
    typedef const typename std::remove_reference<A>::type &Aref;
    typedef std::tuple<A, A> key_t;

    std::unordered_map<key_t, R, pair_hash<A, Hash>> map;
    R default_v;

    key_t mk_entry(Aref lhs, Aref rhs) {
        if (!commutative || Hash{}(lhs) <= Hash{}(rhs)) {
            return std::make_tuple(lhs, rhs);
        } else {
            return std::make_tuple(rhs, lhs);
        }
    }

  public:
    cache(R default_value) : map(), default_v(default_value) {}

    bool contains(Aref lhs, Aref rhs) {
        key_t entry = mk_entry(lhs, rhs);
        auto found = map.find(entry);
        return found != map.end();
    }

    R lookup(Aref lhs, Aref rhs) {
        key_t entry = mk_entry(lhs, rhs);
        auto found = map.find(entry);
        if (found != map.end()) {
            return found->second;
        } else {
            return default_v;
        }
    }

    void insert(Aref lhs, Aref rhs, R result) {
        key_t entry = mk_entry(lhs, rhs);
        map.emplace(entry, result);
    }
};

} // namespace mtidd
