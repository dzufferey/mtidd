#include "mtidd/mtidd.h"

#include <iostream>
#include <random>
#include <chrono>

//tell Catch to provide a main (only once pre cpp)
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

using namespace std;

namespace mtidd {

  TEST_CASE("default elements") {
    idd_manager<int, bool> mngr;
    REQUIRE(mngr.top() == mngr.top());
    REQUIRE(mngr.bottom() == mngr.bottom());
    REQUIRE(mngr.top() != mngr.bottom());
    REQUIRE(mngr.top().equals_structural(mngr.top()));
    REQUIRE(mngr.bottom().equals_structural(mngr.bottom()));
    REQUIRE(!mngr.top().equals_structural(mngr.bottom()));
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
    REQUIRE(dd.equals_structural(dd_copy));
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
    REQUIRE(mngr.bottom().compare(dd1) == Smaller);
    REQUIRE(mngr.bottom().compare(dd2) == Smaller);
    REQUIRE(dd1.compare(mngr.bottom()) == Greater);
    REQUIRE(dd2.compare(mngr.bottom()) == Greater);
    REQUIRE(mngr.top().compare(dd1) == Greater);
    REQUIRE(mngr.top().compare(dd2) == Greater);
    REQUIRE(dd1.compare(mngr.top()) == Smaller);
    REQUIRE(dd2.compare(mngr.top()) == Smaller);
    REQUIRE(dd1.compare(dd2) == Different);
    REQUIRE(dd2.compare(dd1) == Different);
    //
    idd<int, bool> const & dd_inter = dd1 & dd2;
    //cout << endl << "dd_inter" << endl;
    //dd_inter.print(cout);
    REQUIRE(mngr.bottom() == dd_inter);
    REQUIRE(dd_inter.compare(dd1) == Smaller);
    REQUIRE(dd1.compare(dd_inter) == Greater);
    REQUIRE(dd_inter.compare(dd2) == Smaller);
    REQUIRE(dd2.compare(dd_inter) == Greater);
    REQUIRE(mngr.bottom().compare(dd_inter) == Equal);
    //
    idd<int, bool> const & dd_union = dd1 | dd2;
    //cout << endl << "dd_union" << endl;
    //dd_union.print(cout);
    REQUIRE(mngr.top() == dd_union);
    REQUIRE(dd_union.compare(dd1) == Greater);
    REQUIRE(dd1.compare(dd_union) == Smaller);
    REQUIRE(dd_union.compare(dd2) == Greater);
    REQUIRE(dd2.compare(dd_union) == Smaller);
    REQUIRE(mngr.top().compare(dd_union) == Equal);
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
    REQUIRE(mngr.bottom().compare(dd1) == Smaller);
    REQUIRE(mngr.bottom().compare(dd2) == Smaller);
    REQUIRE(dd1.compare(mngr.bottom()) == Greater);
    REQUIRE(dd2.compare(mngr.bottom()) == Greater);
    REQUIRE(mngr.top().compare(dd1) == Greater);
    REQUIRE(mngr.top().compare(dd2) == Greater);
    REQUIRE(dd1.compare(mngr.top()) == Smaller);
    REQUIRE(dd2.compare(mngr.top()) == Smaller);
    REQUIRE(dd1.compare(dd2) == Different);
    REQUIRE(dd2.compare(dd1) == Different);
    //
    idd<int, bool> const & dd_inter = dd1 & dd2;
    REQUIRE(mngr.bottom() == dd_inter);
    REQUIRE(mngr.bottom() == dd_inter);
    REQUIRE(dd_inter.compare(dd1) == Smaller);
    REQUIRE(dd1.compare(dd_inter) == Greater);
    REQUIRE(dd_inter.compare(dd2) == Smaller);
    REQUIRE(dd2.compare(dd_inter) == Greater);
    REQUIRE(mngr.bottom().compare(dd_inter) == Equal);
    //
    idd<int, bool> const & dd_union = dd1 | dd2;
    REQUIRE(mngr.top() == dd_union);
    REQUIRE(dd_union.compare(dd1) == Greater);
    REQUIRE(dd1.compare(dd_union) == Smaller);
    REQUIRE(dd_union.compare(dd2) == Greater);
    REQUIRE(dd2.compare(dd_union) == Smaller);
    REQUIRE(mngr.top().compare(dd_union) == Equal);
  }

  TEST_CASE("traverse caching") {
    idd_manager<int, bool> mngr;
    mngr.internalize_variable(0);
    map<int, interval> box;
    box[0] = make_tuple(-10, Closed, 10, Closed);
    idd<int, bool> const & dd1 = mngr.from_box(box, true, false);
    int count = 0;
    function<void (const idd<int, bool, lattice<bool>> *)> traverser = [&](const idd<int, bool, lattice<bool>>*) { count++; };
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
    mngr.internalize_variable(0);
    mngr.internalize_variable(1);
    mngr.internalize_variable(2);
    mngr.internalize_variable(3);

    map<int, interval> idd_box;
    idd_box[0] = make_tuple(-2, Closed, 2, Closed);
    idd_box[1] = make_tuple(-4, Closed, 4, Closed);
    idd_box[2] = make_tuple(-6, Closed, 6, Closed);
    idd_box[3] = make_tuple(-8, Closed, 8, Closed);
    idd<int, int> const& dd = mngr.from_box(idd_box, 999, -1);

    map<int, interval> test_box;
    test_box[0] = make_tuple(-100, Closed, 100, Closed);
    test_box[1] = make_tuple(-100, Closed, 100, Closed);
    test_box[2] = make_tuple(-100, Closed, 100, Closed);
    test_box[3] = make_tuple(-100, Closed, 100, Closed);

    /*
    test_box[0] = make_tuple(-1, Closed, 1, Closed);
    test_box[1] = make_tuple(-1, Closed, 1, Closed);
    test_box[2] = make_tuple(-1, Closed, 1, Closed);
    test_box[3] = make_tuple(-1, Closed, 1, Closed);
    */

    unordered_set<int> contained = dd.inf_terminal_cover(test_box);
    REQUIRE(contained.size() == 2);
    REQUIRE(contained.count(-1) > 0);
    REQUIRE(contained.count(999) > 0);

  }

  bool next_bool(uniform_int_distribution<int> & dist, default_random_engine & gen) {
    int n = dist(gen);
    return n == 0;
  }

  interval_boundary next_ib(uniform_int_distribution<int> & dist, default_random_engine & gen) {
    return next_bool(dist, gen) ? Open : Closed;
  }

  void fill_box(int dims,
                map<int, interval> & box,
                uniform_real_distribution<double> & rdist,
                uniform_int_distribution<int> & idist,
                default_random_engine & gen) {
    for (int i = 0; i < dims; i++) {
      double lb = rdist(gen);
      if (next_bool(idist, gen)) {
        lb *= -1;
      }
      double ub = lb + rdist(gen);
      box[i] = make_tuple(lb, next_ib(idist, gen), ub, next_ib(idist, gen));
    }
  }

  TEST_CASE("pseudo random operations") {
    const int dims = 100;
    const int ops = 10000;
    const int release_period = 100;

    default_random_engine gen(3141526535);
    uniform_real_distribution<double> rdist(0.0,1000.0);
    uniform_int_distribution<int> idist(0,1); //boolean

    idd_manager<int, bool> mngr;

    for (int i = 0; i < dims; i++) {
      int idx = mngr.internalize_variable(0);
      REQUIRE(idx >= 0);
      REQUIRE(idx < dims);
    }

    using namespace std::chrono;
    steady_clock::time_point t1 = steady_clock::now();

    map<int, interval> box;
    fill_box(dims, box, rdist, idist, gen);
    idd<int, bool> const * dd = &(mngr.from_box(box, true, false));

    for (int i = 0; i < ops; i++) {
      fill_box(dims, box, rdist, idist, gen);
      idd<int, bool> const & tmp = mngr.from_box(box, true, false);
      if (next_bool(idist, gen)) {
        dd = &((*dd) & tmp);
      } else {
        dd = &((*dd) | tmp);
      }
      if (i % release_period == 0) {
        mngr.release_except(*dd);
      }
    }

    steady_clock::time_point t2 = steady_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    cout << "random test with " << dims << " dimensions and " << ops << " operations took " << time_span.count() << " seconds." << endl;
  }

  TEST_CASE("partial_covers") {
    cout << "partial covers:" << endl;

    idd_manager<int, bool> mngr;
    mngr.internalize_variable(0);
    mngr.internalize_variable(1);
    mngr.internalize_variable(2);

    map<int, interval> base_box;
    base_box[0] = make_tuple(-10, Closed, 10, Closed);
    base_box[1] = make_tuple(-20, Closed, 20, Closed);
    base_box[2] = make_tuple(-30, Closed, 30, Closed);
    idd<int, bool> const & dd1 = mngr.from_box(base_box, true, false);

    map<int, interval> box1;
    box1[0] = make_tuple(0, Closed, 50, Closed);
    box1[1] = make_tuple(0, Closed, 100, Closed);
    box1[2] = make_tuple(0, Closed, 150, Closed);

    map<int, interval> lhs_overlaps1;
    map<int, interval> rhs_overlaps1;
    dd1.partial_overlaps(lhs_overlaps1, rhs_overlaps1, box1);

    cout << "LHS:" << endl;
    for (auto iterator = lhs_overlaps1.begin(); iterator != lhs_overlaps1.end(); iterator++) {
      interval intv = iterator->second;
      cout << intv << endl;
    }

    cout << "RHS:" << endl;
    for (auto iterator = rhs_overlaps1.begin(); iterator != rhs_overlaps1.end(); iterator++) {
      interval intv = iterator->second;
      cout << intv << endl;
    }

    std::cout << "-----" << std::endl;
    //

    map<int, interval> box2;
    box2[0] = make_tuple(-50, Closed, 10, Closed);
    box2[1] = make_tuple(-100, Closed, 10, Closed);
    box2[2] = make_tuple(-150, Closed, 10, Closed);

    map<int, interval> lhs_overlaps2;
    map<int, interval> rhs_overlaps2;
    dd1.partial_overlaps(lhs_overlaps2, rhs_overlaps2, box2);

    cout << "LHS 1:" << endl;
    for (auto iterator = lhs_overlaps2.begin(); iterator != lhs_overlaps2.end(); iterator++) {
      interval intv = iterator->second;
      cout << intv << endl;
    }

    cout << "RHS 2:" << endl;
    for (auto iterator = rhs_overlaps2.begin(); iterator != rhs_overlaps2.end(); iterator++) {
      interval intv = iterator->second;
      cout << intv << endl;
    }

  }

}
