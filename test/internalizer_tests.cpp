#include "mtidd.h"

//tell Catch to provide a main (only once pre cpp)
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace std;

namespace mtidd {
  
  TEST_CASE("int internalizer") {
    internalizer<int> it;
    REQUIRE(!it.contains(0));
    REQUIRE(it.number_of_elements() == 0);
    int zero_idx = it.internalize(0);
    REQUIRE( it.contains(0));
    REQUIRE(!it.contains(1));
    REQUIRE(it.index(0) == zero_idx);
    REQUIRE(it.at(zero_idx) == 0);
    REQUIRE(it.number_of_elements() == 1);
    int one_idx = it.internalize(1);
    REQUIRE( it.contains(0));
    REQUIRE( it.contains(1));
    REQUIRE(it.index(0) == zero_idx);
    REQUIRE(it.at(zero_idx) == 0);
    REQUIRE(it.index(1) == one_idx);
    REQUIRE(it.at(one_idx) == 1);
    REQUIRE(it.number_of_elements() == 2);
    it.clear();
    REQUIRE(it.number_of_elements() == 0);
    REQUIRE(!it.contains(0));
    REQUIRE(!it.contains(1));
  }

}
