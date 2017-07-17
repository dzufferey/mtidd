#include "partition_iterator.h"

#include <iostream>

//tell Catch to provide a main (only once pre cpp)
#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

using namespace std;

namespace mtidd {

  TEST_CASE("000") {
    int a = 0;
    int b = 1;
    int c = 2;
    int d = 3;
    partition<int> p = new_partition(&a);
    interval it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p, it, &b);
    it = make_tuple(-15, Closed, -5, Closed);
    insert_partition(p, it, &c);
    it = make_tuple(0, Closed, 0, Closed);
    insert_partition(p, it, &d);
    auto itr = partition_iterator<int>(p);
    auto end = partition_iterator<int>::end(p);
    REQUIRE(itr != end);
    REQUIRE(**itr == a);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == c);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == b);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == d);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == b);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == a);
    ++itr;
    REQUIRE(itr == end);

    it = make_tuple(-10, Closed, 10, Closed);
    itr = partition_iterator<int>::covers(p, it);
    REQUIRE(itr != end);
    REQUIRE(**itr == c);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == b);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == d);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == b);
    ++itr;
    REQUIRE(itr == end);

    itr = partition_iterator<int>::covered_by(p, it);
    REQUIRE(itr != end);
    REQUIRE(**itr == b);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == d);
    ++itr;
    REQUIRE(itr != end);
    REQUIRE(**itr == b);
    ++itr;
    REQUIRE(itr == end); 

  }

}
