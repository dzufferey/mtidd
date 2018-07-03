#include "mtidd/mtidd.h"

//tell Catch to provide a main (only once pre cpp)
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

using namespace std;

namespace mtidd {
  
  TEST_CASE("simple") {
    cache<int, bool> c(false);
    REQUIRE(!c.contains(0,0));
    REQUIRE(c.lookup(0,0) == false);
    c.insert(0, 0, true);
    REQUIRE(c.contains(0,0));
    REQUIRE(!c.contains(1,1));
    REQUIRE(c.lookup(0,0) == true);
  }

  TEST_CASE("simple non-commutative") {
    cache<int, bool, false> c(false);
    REQUIRE(!c.contains(0,1));
    c.insert(0, 1, true);
    REQUIRE(!c.contains(1,0));
    REQUIRE(c.contains(0,1));
    REQUIRE(!c.contains(1,1));
    REQUIRE(c.lookup(0,1) == true);
    REQUIRE(c.lookup(1,0) == false);
  }

  TEST_CASE("simple commutative") {
    cache<int, bool, true> c(false);
    REQUIRE(!c.contains(0,1));
    c.insert(0, 1, true);
    REQUIRE(c.contains(1,0));
    REQUIRE(c.contains(0,1));
    REQUIRE(!c.contains(1,1));
    REQUIRE(c.lookup(1,0) == true);
  }

}
