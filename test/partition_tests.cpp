#include "mtidd/mtidd.hpp"

#include <iostream>
#include <memory>

// tell Catch to provide a main (only once pre cpp)
#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"

using namespace std;

namespace mtidd {

TEST_CASE("insert 0") {
    auto a = make_shared<const int>(0);
    auto b = make_shared<const int>(1);
    auto c = make_shared<const int>(2);
    auto d = make_shared<const int>(3);
    auto e = make_shared<const int>(5);
    partition<int> p = new_partition(a);
    REQUIRE(*lookup_partition(p, 1234) == *a);
    interval it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p, it, b);
    REQUIRE(*lookup_partition(p, 1234) == *a);
    REQUIRE(*lookup_partition(p, -10) == *a);
    REQUIRE(*lookup_partition(p, 0) == *b);
    REQUIRE(*lookup_partition(p, 10) == *b);
    insert_partition(p, it, c);
    REQUIRE(*lookup_partition(p, 1234) == *a);
    REQUIRE(*lookup_partition(p, -10) == *a);
    REQUIRE(*lookup_partition(p, 0) == *c);
    REQUIRE(*lookup_partition(p, 10) == *c);
    it = make_tuple(-15, Closed, -5, Closed);
    insert_partition(p, it, d);
    REQUIRE(*lookup_partition(p, -16) == *a);
    REQUIRE(*lookup_partition(p, -15) == *d);
    REQUIRE(*lookup_partition(p, -5) == *d);
    REQUIRE(*lookup_partition(p, -4) == *c);
    REQUIRE(*lookup_partition(p, 0) == *c);
    REQUIRE(*lookup_partition(p, 10) == *c);
    REQUIRE(*lookup_partition(p, 1234) == *a);
    it = make_tuple(0, Closed, 0, Closed);
    insert_partition(p, it, e);
    REQUIRE(*lookup_partition(p, -16) == *a);
    REQUIRE(*lookup_partition(p, -15) == *d);
    REQUIRE(*lookup_partition(p, -5) == *d);
    REQUIRE(*lookup_partition(p, -4) == *c);
    REQUIRE(*lookup_partition(p, 0) == *e);
    REQUIRE(*lookup_partition(p, 10) == *c);
    REQUIRE(*lookup_partition(p, 1234) == *a);
}

TEST_CASE("map") {
    auto a1 = make_shared<const int>(0);
    auto b1 = make_shared<const int>(1);
    auto c1 = make_shared<const int>(2);
    partition<int> p1 = new_partition(a1);
    //
    auto a2 = make_shared<const int>(4);
    auto b2 = make_shared<const int>(5);
    auto c2 = make_shared<const int>(6);
    partition<int> p2 = new_partition(a2);
    //
    interval it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p1, it, b1);
    it = make_tuple(0, Closed, 20, Closed);
    insert_partition(p1, it, c1);
    //
    function<shared_ptr<const int>(shared_ptr<const int>)> mapper =
        [&](shared_ptr<const int> x) -> shared_ptr<const int> {
        if (x == a1)
            return a2;
        else if (x == b1)
            return b2;
        else
            return c2;
    };
    map_partition<int>(p2, p1, mapper);
    //
    REQUIRE(*lookup_partition(p1, -10) == *a1);
    REQUIRE(*lookup_partition(p2, -10) == *a2);
    REQUIRE(*lookup_partition(p1, -9) == *b1);
    REQUIRE(*lookup_partition(p2, -9) == *b2);
    REQUIRE(*lookup_partition(p1, 0) == *c1);
    REQUIRE(*lookup_partition(p2, 0) == *c2);
    REQUIRE(*lookup_partition(p1, 20) == *c1);
    REQUIRE(*lookup_partition(p2, 20) == *c2);
    REQUIRE(*lookup_partition(p1, 21) == *a1);
    REQUIRE(*lookup_partition(p2, 21) == *a2);
}

TEST_CASE("insert 1") {
    auto a2 = make_shared<const int>(4);
    auto b2 = make_shared<const int>(5);
    auto c2 = make_shared<const int>(6);
    partition<int> p2 = new_partition(a2);
    // cerr << "==========" << endl;
    interval it = make_tuple(-20, Open, 0, Open);
    insert_partition(p2, it, b2);
    // cerr << "----------" << endl;
    it = make_tuple(0, Closed, 20, Closed);
    insert_partition(p2, it, c2);
    // cerr << "----------" << endl;
    // cerr << p2 << endl;
    REQUIRE(p2.size() == 4);
}

TEST_CASE("merge") {
    auto a1 = make_shared<const int>(0);
    auto b1 = make_shared<const int>(1);
    auto c1 = make_shared<const int>(2);
    partition<int> p1 = new_partition(a1);
    interval it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p1, it, b1);
    it = make_tuple(0, Closed, 20, Closed);
    insert_partition(p1, it, c1);
    //
    auto a2 = make_shared<const int>(4);
    auto b2 = make_shared<const int>(5);
    auto c2 = make_shared<const int>(6);
    partition<int> p2 = new_partition(a2);
    it = make_tuple(-20, Open, 0, Open);
    insert_partition(p2, it, b2);
    it = make_tuple(0, Closed, 20, Closed);
    insert_partition(p2, it, c2);
    //
    auto a3 = make_shared<const int>(0);
    int cnt = -1;
    partition<int> p3 = new_partition(a3);
    function<shared_ptr<const int>(shared_ptr<const int>, shared_ptr<const int>)> merger =
        [&](shared_ptr<const int> x, shared_ptr<const int> y) -> shared_ptr<const int> {
        cnt++;
        return make_shared<const int>(*x + *y);
    };
    merge_partition<int>(p3, p2, p1, merger);
    // cerr << p1 << endl;
    // cerr << p2 << endl;
    // cerr << p3 << endl;
    REQUIRE(cnt == 4);
    REQUIRE(p3.size() == 5);
    REQUIRE(*lookup_partition(p3, -50) == *a1 + *a2);
    REQUIRE(*lookup_partition(p3, -15) == *a1 + *b2);
    REQUIRE(*lookup_partition(p3, -9) == *b1 + *b2);
    REQUIRE(*lookup_partition(p3, 0) == *c1 + *c2);
    REQUIRE(*lookup_partition(p3, 30) == *a1 + *a2);
}

TEST_CASE("hash") {
    function<size_t(const int &)> hasher = [](const int &x) -> size_t {
        cout << "x:" << x << endl;
        return x;
    };
    //
    auto a1 = make_shared<const int>(0);
    auto b1 = make_shared<const int>(1);
    auto c1 = make_shared<const int>(2);
    partition<int> p1 = new_partition(a1);
    interval it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p1, it, b1);
    it = make_tuple(0, Closed, 20, Closed);
    insert_partition(p1, it, c1);
    //
    auto a2 = make_shared<const int>(4);
    auto b2 = make_shared<const int>(5);
    auto c2 = make_shared<const int>(6);
    partition<int> p2 = new_partition(a2);
    //
    REQUIRE(hash_partition(p1, hasher) != hash_partition(p2, hasher));
    //
    it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p2, it, b2);
    it = make_tuple(0, Closed, 20, Closed);
    insert_partition(p2, it, c2);
    REQUIRE(hash_partition(p1, hasher) != hash_partition(p2, hasher));
    //
    partition<int> p3 = new_partition(a1);
    it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p3, it, b1);
    it = make_tuple(0, Closed, 20, Closed);
    insert_partition(p3, it, c1);
    REQUIRE(hash_partition(p1, hasher) == hash_partition(p3, hasher));
    //
    auto a4 = make_shared<const int>(0);
    auto b4 = make_shared<const int>(1);
    auto c4 = make_shared<const int>(2);
    partition<int> p4 = new_partition(a4);
    it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p4, it, b4);
    it = make_tuple(0, Closed, 20, Closed);
    insert_partition(p4, it, c4);
    REQUIRE(hash_partition(p1, hasher) == hash_partition(p4, hasher));
}

TEST_CASE("filter") {
    cout << "--- filter" << endl;
    auto a = make_shared<const int>(0);
    auto b = make_shared<const int>(1);
    auto c = make_shared<const int>(2);
    auto d = make_shared<const int>(3);
    auto e = make_shared<const int>(5);
    partition<int> p = new_partition(a);

    interval it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p, it, b);
    insert_partition(p, it, c);
    it = make_tuple(-15, Closed, -5, Closed);
    insert_partition(p, it, d);
    it = make_tuple(0, Closed, 0, Closed);
    insert_partition(p, it, e);
    cout << "Partition: " << p << endl;

    function<bool(const int &)> is_even = [&](const int &x) -> bool { return x % 2 == 0; };

    list<interval> results;
    filter_partition(results, p, is_even);

    cout << "filtered:" << endl;
    for (auto c : results)
        cout << c << endl;
}

TEST_CASE("foldl") {
    cout << "--- foldl" << endl;
    auto a = make_shared<const int>(1);
    auto b = make_shared<const int>(2);
    auto c = make_shared<const int>(3);
    auto d = make_shared<const int>(4);
    auto e = make_shared<const int>(-5);
    partition<int> p = new_partition(a);

    interval it = make_tuple(-10, Open, 10, Closed);
    insert_partition(p, it, b);
    insert_partition(p, it, c);
    it = make_tuple(-15, Closed, -5, Closed);
    insert_partition(p, it, d);
    it = make_tuple(0, Closed, 0, Closed);
    insert_partition(p, it, e);
    cout << "Partition: " << p << endl;

    function<int(int, const int &)> add = [&](int acc, const int &x) -> int { return acc + x; };

    int result = 0;
    foldl_partition(result, p, add);
    cout << "foldl result: " << result << endl;
}

} // namespace mtidd
