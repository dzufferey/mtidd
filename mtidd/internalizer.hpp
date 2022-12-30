#pragma once

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

namespace mtidd {

// takes, stores object, and give an int reference
// the goal is to put all the object in a single place and not leak memory
// implicitely this also gives an order on the stored elements
template <class T, class Hash = std::hash<T>, class Equal = std::equal_to<T>> class internalizer {
  private:
    std::unordered_map<T, int, Hash, Equal> by_value;
    std::vector<T> by_index;

  public:
    int index(T const &v) const { return by_value.at(v); }

    T at(int i) const { return by_index.at(i); }

    int compare(T const &v1, T const &v2) const { return by_value.at(v1) - by_value.at(v2); }

    int number_of_elements() const {
        assert(by_value.size() == by_index.size());
        return by_index.size();
    }

    int internalize(T const &v) {
        auto found = by_value.find(v);
        if (found == by_value.end()) {
            int i = by_index.size();
            // by_index.emplace_back(v); this needs C++14
            by_index.push_back(v);
            by_value.emplace(v, i);
            return i;
        } else {
            return found->second;
        }
    }

    void permute(std::map<int, int> const &permutation) {
        // update by_index
        std::vector<T> old_indices = by_index; // FIXME twice the memory during this method...
        by_index.clear();
        auto n = old_indices.size();
        by_index.resize(n);
        for (int i = 0; i < n; ++i) {
            int new_index = i;
            if (permutation.count(i) > 0) {
                new_index = permutation.at(i);
            }
            by_index[new_index] = old_indices[i];
        }
        // update by_value
        for (auto it = by_value.begin(); it != by_value.end(); ++it) {
            int index = it->first;
            if (permutation.count(index) > 0) {
                it->second = permutation.at(index);
            }
        }
    }

    void clear() {
        by_index.clear();
        by_value.clear();
    }

    // this invalidates the indices
    void clear(std::set<T> const &values_to_keep) {
        clear();
        for (auto it = values_to_keep.begin(); it != values_to_keep.end(); ++it) {
            internalize(*it);
        }
    }

    bool contains(T const &v) const { return by_value.find(v) != by_value.end(); }
};

} // namespace mtidd
