#include "partition.h"

#include <iostream>

//tell Catch to provide a main (only once pre cpp)
#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

using namespace std;

namespace mtidd {

  TEST_CASE("insert 0") {
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
    REQUIRE( *lookup_partition(p, -16) == a);
    REQUIRE( *lookup_partition(p, -15) == d);
    REQUIRE( *lookup_partition(p, -5) == d);
    REQUIRE( *lookup_partition(p, -4) == c);
    REQUIRE( *lookup_partition(p, 0) == e);
    REQUIRE( *lookup_partition(p, 10) == c);
    REQUIRE( *lookup_partition(p, 1234) == a);
  }
  
  TEST_CASE("map") {
    int a1 = 0;
    int b1 = 1;
    int c1 = 2;
    partition<int> p1 = new_partition(&a1);
    //
    int a2 = 4;
    int b2 = 5;
    int c2 = 6;
    partition<int> p2 = new_partition(&a2);
    //
    interval it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p1, it, &b1);
    it = make_tuple(0, Closed, 20, Closed);
    insert_partition(p1, it, &c1);
    //
    function<int* (const int *)> mapper = [&](const int* x) -> int* {
        if (x == &a1) return &a2;
        else if (x == &b1) return &b2;
        else return &c2;
    };
    map_partition<int>(p2, p1, mapper);
    //
    REQUIRE( *lookup_partition(p1, -10) == a1);
    REQUIRE( *lookup_partition(p2, -10) == a2);
    REQUIRE( *lookup_partition(p1, -9) == b1);
    REQUIRE( *lookup_partition(p2, -9) == b2);
    REQUIRE( *lookup_partition(p1,  0) == c1);
    REQUIRE( *lookup_partition(p2,  0) == c2);
    REQUIRE( *lookup_partition(p1, 20) == c1);
    REQUIRE( *lookup_partition(p2, 20) == c2);
    REQUIRE( *lookup_partition(p1, 21) == a1);
    REQUIRE( *lookup_partition(p2, 21) == a2);
  }

  TEST_CASE("insert 1") {
    int a2 = 4;
    int b2 = 5;
    int c2 = 6;
    partition<int> p2 = new_partition(&a2);
    //cerr << "==========" << endl;
    interval it = make_tuple(-20, Open, 0, Open);
    insert_partition(p2, it, &b2);
    //cerr << "----------" << endl;
    it = make_tuple(0, Closed, 20, Closed);
    insert_partition(p2, it, &c2);
    //cerr << "----------" << endl;
    //cerr << p2 << endl;
    REQUIRE( p2.size() == 4);
  }

  TEST_CASE("merge") {
    int a1 = 0;
    int b1 = 1;
    int c1 = 2;
    partition<int> p1 = new_partition(&a1);
    interval it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p1, it, &b1);
    it = make_tuple(0, Closed, 20, Closed);
    insert_partition(p1, it, &c1);
    //
    int a2 = 4;
    int b2 = 5;
    int c2 = 6;
    partition<int> p2 = new_partition(&a2);
    it = make_tuple(-20, Open, 0, Open);
    insert_partition(p2, it, &b2);
    it = make_tuple(0, Closed, 20, Closed);
    insert_partition(p2, it, &c2);
    //
    int a3 = 0;
    int cnt = -1;
    int res[20];
    for (int i = 0; i < 20; i++) res[i] = 0;
    partition<int> p3 = new_partition(&a3);
    function<int* (const int *, const int *)> merger = [&](const int* x, const int* y) -> int* {
      cnt++;
      res[cnt] = *x + *y;
      //cerr << "res[" << cnt << "] = " << *x << " + " << *y << endl;
      return &res[cnt];
    };
    merge_partition <int>(p3, p2, p1, merger);
    //cerr << p1 << endl;
    //cerr << p2 << endl;
    //cerr << p3 << endl;
    REQUIRE( cnt == 4);
    REQUIRE( p3.size() == 5);
    REQUIRE( *lookup_partition(p3, -50) == a1 + a2);
    REQUIRE( *lookup_partition(p3, -15) == a1 + b2);
    REQUIRE( *lookup_partition(p3,  -9) == b1 + b2);
    REQUIRE( *lookup_partition(p3,   0) == c1 + c2);
    REQUIRE( *lookup_partition(p3,  30) == a1 + a2);
  }

  TEST_CASE("hash") {
    function<size_t (const int *)> hasher = [](const int* x) -> size_t {
      return *x;
    };
    //
    int a1 = 0;
    int b1 = 1;
    int c1 = 2;
    partition<int> p1 = new_partition(&a1);
    interval it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p1, it, &b1);
    it = make_tuple(0, Closed, 20, Closed);
    insert_partition(p1, it, &c1);
    //
    int a2 = 0;
    int b2 = 1;
    int c2 = 2;
    partition<int> p2 = new_partition(&a2);
    //
    REQUIRE(hash_partition(p1, hasher) != hash_partition(p2, hasher));
    //
    it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p2, it, &b2);
    it = make_tuple(0, Closed, 20, Closed);
    insert_partition(p2, it, &c2);
    REQUIRE(hash_partition(p1, hasher) == hash_partition(p2, hasher));
  }

  TEST_CASE("covers") {
    cout << "--- covers" << endl;
    int a = 0;
    int b = 1;
    int c = 2;
    int d = 3;
    int e = 4;
    int f = 5;
    partition<int> p = new_partition(&a);
  
    interval it = make_tuple(-10, Open, 10, Closed);
    it = make_tuple(-20, Open, 20, Closed);
    insert_partition(p, it, &f);
    insert_partition(p, it, &b);
    insert_partition(p, it, &c);
    it = make_tuple(-15, Closed, -5, Closed);
    insert_partition(p, it, &d);
    it = make_tuple(0, Closed, 0, Closed);
    insert_partition(p, it, &e);

    interval test = make_tuple(-10, Closed, 10, Closed);
    // interval test = make_tuple<double, interval_boundary, double, interval_boundary>(-numeric_limits<double>::infinity(), Open, numeric_limits<double>::infinity(), Open);

    list<const int*> cover_contents;
    interval_covers(cover_contents, p, test);
    list<const int*> covered_by_contents;
    interval_covered_by(covered_by_contents, p, test);

    cout << "test interval:" << test << endl;
    cout << "Partition: " << p << endl;
    cout << "interval covers:";
    for (auto c : cover_contents) cout << " " << *c;
    cout << endl;
    cout << "interval covered by:";
    for (auto c : covered_by_contents) cout << " " << *c;
    cout << endl;
 
    list<int> target_covers = {2, 4};
    list<int> non_ptr_covers;
    for (auto it = cover_contents.begin(); it != cover_contents.end(); it++) {
      non_ptr_covers.push_back(**it);
    }
    REQUIRE(target_covers == non_ptr_covers);

    list<int> target_covered_by = {3, 2, 4, 2};
    list<int> non_ptr_covered_by;
    for (auto it = covered_by_contents.begin(); it != covered_by_contents.end(); it++) {
      non_ptr_covered_by.push_back(**it);
    }
    REQUIRE(target_covered_by == non_ptr_covered_by);
 }

  TEST_CASE("filter") {
    cout << "--- filter" << endl;
    int a = 0;
    int b = 1;
    int c = 2;
    int d = 3;
    int e = 4;
    partition<int> p = new_partition(&a);
  
    interval it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p, it, &b);
    insert_partition(p, it, &c);
    it = make_tuple(-15, Closed, -5, Closed);
    insert_partition(p, it, &d);
    it = make_tuple(0, Closed, 0, Closed);
    insert_partition(p, it, &e);
    cout << "Partition: " << p << endl;

    function <bool (const int*)> is_even = [&](const int* x) -> bool {return (*x % 2 == 0);};

    list<interval> results;
    filter_partition(results, p, is_even);

    cout << "filtered:" << endl;
    for (auto c : results) cout << c << endl;
  }
  
  TEST_CASE("foldl") {
    cout << "--- foldl" << endl;
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = -5;
    partition<int> p = new_partition(&a);
  
    interval it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p, it, &b);
    insert_partition(p, it, &c);
    it = make_tuple(-15, Closed, -5, Closed);
    insert_partition(p, it, &d);
    it = make_tuple(0, Closed, 0, Closed);
    insert_partition(p, it, &e);
    cout << "Partition: " << p << endl;

    function <int (int, const int*)> add = [&](int acc, const int* x) -> int {return acc + *x;};

    int result = 0;
    foldl_partition(result, p, add);
    cout << "foldl result: " << result << endl;
  }
}
