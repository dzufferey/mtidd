#include "interval.h"

#include <iostream>
#include <vector>

//tell Catch to provide a main (only once pre cpp)
#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

using std;

namespace mtidd {

  TEST_CASE("dummy") {
    REQUIRE(1+1 == 2);
  }

}
