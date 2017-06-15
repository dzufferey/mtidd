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
    REQUIRE(mngr.top().compare_structural(mngr.top()));
    REQUIRE(mngr.bottom().compare_structural(mngr.bottom()));
    REQUIRE(!mngr.top().compare_structural(mngr.bottom()));
  }

  TEST_CASE("internalize var") {
    idd_manager<int, bool> mngr;
    mngr.internalize_variable(0);
    mngr.internalize_variable(1);
    mngr.internalize_variable(2);
    mngr.internalize_variable(3);
    REQUIRE(mngr.number_of_variables() == 4);
  }

  TEST_CASE("construct from terminal") {
    idd_manager<int, bool> mngr;
    auto dd1 = mngr.from_terminal(true);
    auto dd2 = mngr.from_terminal(false);
    REQUIRE(dd1 == mngr.top());
    REQUIRE(dd2 == mngr.bottom());
    REQUIRE(true);
  }

  TEST_CASE("construct from box") {
    idd_manager<int, bool> mngr;
    mngr.internalize_variable(0);
    mngr.internalize_variable(1);
    map<int, interval> box;
    box[0] = make_tuple(-10, Closed, 10, Closed);
    box[1] = make_tuple(-20, Closed, 10, Closed);
    auto dd = mngr.from_box(box, true, false);
    REQUIRE(true);
  }

}
