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

  TEST_CASE("internalize") {
    idd_manager<int, int> mngr;
    int idx0 = mngr.internalize_variable(0);
    int idx1 = mngr.internalize_variable(1);
    int idx2 = mngr.internalize_variable(2);
    int idx3 = mngr.internalize_variable(3);
    REQUIRE(mngr.number_of_variables() == 4);
    REQUIRE(mngr.variable_at(idx0) == 0);
    REQUIRE(mngr.variable_at(idx1) == 1);
    REQUIRE(mngr.variable_at(idx2) == 2);
    REQUIRE(mngr.variable_at(idx3) == 3);
    idx0 = mngr.internalize_terminal(true);
    idx1 = mngr.internalize_terminal(false);
    REQUIRE(mngr.terminal_at(idx0) == true);
    REQUIRE(mngr.terminal_at(idx1) == false);
  }

  TEST_CASE("construct from terminal") {
    idd_manager<int, bool> mngr;
    idd<int, bool> const & dd1 = mngr.from_terminal(true);
    idd<int, bool> const & dd2 = mngr.from_terminal(false);
    REQUIRE(dd1 == mngr.top());
    REQUIRE(dd2 == mngr.bottom());
  }

  TEST_CASE("construct from box") {
    idd_manager<int, bool> mngr;
    mngr.internalize_variable(0);
    mngr.internalize_variable(1);
    map<int, interval> box;
    box[0] = make_tuple(-10, Closed, 10, Closed);
    box[1] = make_tuple(-20, Closed, 10, Closed);
    idd<int, bool> const & dd = mngr.from_box(box, true, false);
    map<int, double> point;
    point[0] = -100;
    point[1] = 0;
    REQUIRE(!dd.lookup(point));
    point[0] = 0;
    point[1] = 0;
    REQUIRE( dd.lookup(point));
    point[0] = -10;
    point[1] = 10;
    REQUIRE( dd.lookup(point));
    idd<int, bool> const & dd_copy = mngr.from_box(box, true, false);
    REQUIRE(dd == dd_copy);
    REQUIRE(dd.compare_structural(dd_copy));
    REQUIRE(dd.hash() == dd_copy.hash());
    idd<int, bool> const & dd_inter = dd & dd;
    REQUIRE(dd == dd_inter);
    idd<int, bool> const & dd_union = dd | dd;
    REQUIRE(dd == dd_union);
  }

  TEST_CASE("operations 00") {
    idd_manager<int, bool> mngr;
    mngr.internalize_variable(0);
    map<int, interval> box;
    box[0] = make_tuple(-10, Closed, 10, Closed);
    idd<int, bool> const & dd1 = mngr.from_box(box, true, false);
    idd<int, bool> const & dd2 = mngr.from_box(box, false, true);
    //cout << "dd1" << endl;
    //dd1.print(cout);
    //cout << endl << "dd2" << endl;
    //dd2.print(cout);
    idd<int, bool> const & dd_inter = dd1 & dd2;
    //cout << endl << "dd_inter" << endl;
    //dd_inter.print(cout);
    REQUIRE(mngr.bottom() == dd_inter);
    idd<int, bool> const & dd_union = dd1 | dd2;
    //cout << endl << "dd_union" << endl;
    //dd_union.print(cout);
    REQUIRE(mngr.top() == dd_union);
  }

  TEST_CASE("operations 01") {
    idd_manager<int, bool> mngr;
    mngr.internalize_variable(0);
    mngr.internalize_variable(1);
    map<int, interval> box;
    box[0] = make_tuple(-10, Closed, 10, Closed);
    box[1] = make_tuple(-20, Closed, 10, Closed);
    idd<int, bool> const & dd1 = mngr.from_box(box, true, false);
    idd<int, bool> const & dd2 = mngr.from_box(box, false, true);
    idd<int, bool> const & dd_inter = dd1 & dd2;
    REQUIRE(mngr.bottom() == dd_inter);
    idd<int, bool> const & dd_union = dd1 | dd2;
    REQUIRE(mngr.top() == dd_union);
  }

  TEST_CASE("traverse caching") {
    idd_manager<int, bool> mngr;
    mngr.internalize_variable(0);
    map<int, interval> box;
    box[0] = make_tuple(-10, Closed, 10, Closed);
    idd<int, bool> const & dd1 = mngr.from_box(box, true, false);
    int count = 0;
    function<void (const idd<int, bool, lattice<bool>> *)> traverser = [&](const idd<int, bool, lattice<bool>>* x) { count++; };
    dd1.traverse(traverser);
    REQUIRE(count == 3);
    count = 0;
    dd1.traverse_all(traverser);
    REQUIRE(count == 4);
  }

  TEST_CASE("traverse combine caching") {
    idd_manager<int, bool> mngr;
    mngr.internalize_variable(0);
    map<int, interval> box;
    box[0] = make_tuple(-10, Closed, 10, Closed);
    idd<int, bool> const & dd1 = mngr.from_box(box, true, false);
    int count = 0;
    function<bool (bool const &, bool const &)> combiner = [&](bool const & x, bool const & y) {
      count++;
      return x ^ y;
    };
    idd<int, bool> const & dd2 = dd1.combine(dd1, combiner);
    REQUIRE(count == 2);
    REQUIRE(mngr.bottom() == dd2);
  }

  TEST_CASE("box contains") {
    idd_manager<int, int> mngr;
    int idx0 = mngr.internalize_variable(0);
    int idx1 = mngr.internalize_variable(1);
    int idx2 = mngr.internalize_variable(2);
    int idx3 = mngr.internalize_variable(3);

    std::map<int, interval> idd_box;
    idd_box[0] = make_tuple(-2, Closed, 2, Closed);
    idd_box[1] = make_tuple(-4, Closed, 4, Closed);
    idd_box[2] = make_tuple(-6, Closed, 6, Closed);
    idd_box[3] = make_tuple(-8, Closed, 8, Closed);
    idd<int, int> const& dd = mngr.from_box(idd_box, 999, -1);

    std::map<int, interval> test_box;
    test_box[0] = make_tuple(-100, Closed, 100, Closed);
    test_box[1] = make_tuple(-100, Closed, 100, Closed);
    test_box[2] = make_tuple(-100, Closed, 100, Closed);
    test_box[3] = make_tuple(-100, Closed, 100, Closed);

    /*
    test_box[0] = make_tuple(0, Closed, 100, Closed);
    test_box[1] = make_tuple(0, Closed, 100, Closed);
    test_box[2] = make_tuple(0, Closed, 100, Closed);
    test_box[3] = make_tuple(0, Closed, 100, Closed);
    */

    std::unordered_set<int> contained = dd.boxed_terminals(test_box);
    
    std::cout << "Terminals that are contained:" << std::endl;
    std::cout << contained.size() << std::endl;
    for (auto item : contained) {
        std::cout << item << std::endl;
    }
  }

}
