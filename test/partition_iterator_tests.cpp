#include "mtidd/mtidd.hpp"

#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"

using namespace std;

namespace mtidd {

TEST_CASE("000") {
    auto a = make_shared<const int>(0);
    auto b = make_shared<const int>(1);
    auto c = make_shared<const int>(2);
    auto d = make_shared<const int>(3);
    partition<int> p = new_partition(a);
    interval it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p, it, b);
    it = make_tuple(-15, Closed, -5, Closed);
    insert_partition(p, it, c);
    it = make_tuple(0, Closed, 0, Closed);
    insert_partition(p, it, d);
    auto itr = partition_iterator<int>(p);
    auto end = partition_iterator<int>::end(p);
    REQUIRE(itr != end);
    REQUIRE(**itr == *a);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == *c);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == *b);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == *d);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == *b);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == *a);
    ++itr;
    REQUIRE(itr == end);

    it = make_tuple(-10, Closed, 10, Closed);
    itr = partition_iterator<int>::covers(p, it);
    REQUIRE(itr != end);
    REQUIRE(**itr == *c);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == *b);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == *d);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == *b);
    ++itr;
    REQUIRE(itr == end);

    itr = partition_iterator<int>::covered_by(p, it);
    REQUIRE(itr != end);
    REQUIRE(**itr == *b);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == *d);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == *b);
    ++itr;
    REQUIRE(itr == end);
}

TEST_CASE("covers") {
    auto a = make_shared<const int>(0);
    auto b = make_shared<const int>(1);
    auto c = make_shared<const int>(2);
    auto d = make_shared<const int>(3);
    auto e = make_shared<const int>(4);
    auto f = make_shared<const int>(5);
    partition<int> p = new_partition(a);

    interval it = make_tuple(-10, Open, 10, Closed);
    it = make_tuple(-20, Open, 20, Closed);
    insert_partition(p, it, f);
    insert_partition(p, it, b);
    insert_partition(p, it, c);
    it = make_tuple(-15, Closed, -5, Closed);
    insert_partition(p, it, d);
    it = make_tuple(0, Closed, 0, Closed);
    insert_partition(p, it, e);

    interval test = make_tuple(-10, Closed, 10, Closed);
    // interval test = make_tuple<double, interval_boundary, double,
    // interval_boundary>(-numeric_limits<double>::infinity(), Open,
    // numeric_limits<double>::infinity(), Open);

    // covers
    auto itr = partition_iterator<int>::covers(p, test);
    auto end = partition_iterator<int>::end(p);

    REQUIRE(itr != end);
    REQUIRE(**itr == *d);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == *c);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == *e);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == *c);
    ++itr;
    REQUIRE(itr == end);

    // covered_by
    itr = partition_iterator<int>::covered_by(p, test);

    REQUIRE(itr != end);
    REQUIRE(**itr == *c);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == *e);
    ++itr;
    REQUIRE(itr == end);
}

} // namespace mtidd
