#include "mtidd.h"

//tell Catch to provide a main (only once pre cpp)
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace std;

namespace mtidd {
  
  TEST_CASE("boolean lattice") {
    lattice<bool> b;
    REQUIRE(b.top());
    REQUIRE(!b.bottom());
    REQUIRE( b.least_upper_bound(true, true));
    REQUIRE( b.least_upper_bound(true,false));
    REQUIRE( b.least_upper_bound(false,true));
    REQUIRE(!b.least_upper_bound(false,false));
    REQUIRE( b.greatest_lower_bound(true, true));
    REQUIRE(!b.greatest_lower_bound(true,false));
    REQUIRE(!b.greatest_lower_bound(false,true));
    REQUIRE(!b.greatest_lower_bound(false,false));
    REQUIRE( b.equal(true, true));
    REQUIRE(!b.equal(true,false));
    REQUIRE(!b.equal(false,true));
    REQUIRE( b.equal(false,false));
    REQUIRE(b.compare(true, true) == Equal);
    REQUIRE(b.compare(true,false) == Greater);
    REQUIRE(b.compare(false,true) == Smaller);
    REQUIRE(b.compare(false,false) == Equal);
    REQUIRE(b.hash(true) != b.hash(false));
  }

}
