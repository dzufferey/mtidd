#include "partition.h"

#include <iostream>

//tell Catch to provide a main (only once pre cpp)
#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

using namespace std;

namespace mtidd {

  TEST_CASE("insert") {
    int a = 0;
    int b = 1;
    int c = 2;
    int d = 3;
    int e = 4;
    partition<int> p = new_partition(&a);
    REQUIRE( *lookup_partition(p, 1234) == a);
    interval it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p, it, &b);
    REQUIRE( *lookup_partition(p, 1234) == a);
    REQUIRE( *lookup_partition(p, -10) == a);
    REQUIRE( *lookup_partition(p, 0) == b);
    REQUIRE( *lookup_partition(p, 10) == b);
    insert_partition(p, it, &c);
    REQUIRE( *lookup_partition(p, 1234) == a);
    REQUIRE( *lookup_partition(p, -10) == a);
    REQUIRE( *lookup_partition(p, 0) == c);
    REQUIRE( *lookup_partition(p, 10) == c);
    it = make_tuple(-15, Closed, -5, Closed);
    insert_partition(p, it, &d);
    REQUIRE( *lookup_partition(p, -16) == a);
    REQUIRE( *lookup_partition(p, -15) == d);
    REQUIRE( *lookup_partition(p, -5) == d);
    REQUIRE( *lookup_partition(p, -4) == c);
    REQUIRE( *lookup_partition(p, 0) == c);
    REQUIRE( *lookup_partition(p, 10) == c);
    REQUIRE( *lookup_partition(p, 1234) == a);
    it = make_tuple(0, Closed, 0, Closed);
    insert_partition(p, it, &e);
    //cerr << p << endl;
    REQUIRE( *lookup_partition(p, -16) == a);
    REQUIRE( *lookup_partition(p, -15) == d);
    REQUIRE( *lookup_partition(p, -5) == d);
    REQUIRE( *lookup_partition(p, -4) == c);
    REQUIRE( *lookup_partition(p, 0) == e);
    REQUIRE( *lookup_partition(p, 10) == c);
    REQUIRE( *lookup_partition(p, 1234) == a);
  }

}
