#include "mtidd.h"

#include <iostream>

//tell Catch to provide a main (only once pre cpp)
#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

using namespace std;

namespace mtidd {

  TEST_CASE("default elements") {
    idd_manager<int, bool> mngr;
    REQUIRE(mngr.top() == mngr.top());
    REQUIRE(mngr.bottom() == mngr.bottom());
    REQUIRE(mngr.top() != mngr.bottom());
  }

  TEST_CASE("internalize var") {
    idd_manager<int, bool> mngr;
    mngr.internalize_variable(0);
    mngr.internalize_variable(1);
    mngr.internalize_variable(2);
    mngr.internalize_variable(3);
    REQUIRE(mngr.number_of_variables() == 4);
  }

}
