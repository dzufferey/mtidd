#include "mtidd/mtidd.hpp"

// tell Catch to provide a main (only once pre cpp)
#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"

using namespace std;

namespace mtidd {

TEST_CASE("lattice_compare") {
    REQUIRE(flip(lattice_compare::equivalent) == lattice_compare::equivalent);
    REQUIRE(flip(lattice_compare::unordered) == lattice_compare::unordered);
    REQUIRE(flip(lattice_compare::less) == lattice_compare::greater);
    REQUIRE(flip(lattice_compare::greater) == lattice_compare::less);
    REQUIRE((lattice_compare::equivalent & lattice_compare::equivalent) == lattice_compare::equivalent);
    REQUIRE((lattice_compare::equivalent & lattice_compare::greater) == lattice_compare::greater);
    REQUIRE((lattice_compare::equivalent & lattice_compare::less) == lattice_compare::less);
    REQUIRE((lattice_compare::equivalent & lattice_compare::unordered) == lattice_compare::unordered);
    REQUIRE((lattice_compare::greater & lattice_compare::equivalent) == lattice_compare::greater);
    REQUIRE((lattice_compare::greater & lattice_compare::greater) == lattice_compare::greater);
    REQUIRE((lattice_compare::greater & lattice_compare::less) == lattice_compare::unordered);
    REQUIRE((lattice_compare::greater & lattice_compare::unordered) == lattice_compare::unordered);
    REQUIRE((lattice_compare::less & lattice_compare::equivalent) == lattice_compare::less);
    REQUIRE((lattice_compare::less & lattice_compare::greater) == lattice_compare::unordered);
    REQUIRE((lattice_compare::less & lattice_compare::less) == lattice_compare::less);
    REQUIRE((lattice_compare::less & lattice_compare::unordered) == lattice_compare::unordered);
    REQUIRE((lattice_compare::unordered & lattice_compare::equivalent) == lattice_compare::unordered);
    REQUIRE((lattice_compare::unordered & lattice_compare::greater) == lattice_compare::unordered);
    REQUIRE((lattice_compare::unordered & lattice_compare::less) == lattice_compare::unordered);
    REQUIRE((lattice_compare::unordered & lattice_compare::unordered) == lattice_compare::unordered);
    REQUIRE((lattice_compare::equivalent | lattice_compare::equivalent) == lattice_compare::equivalent);
    REQUIRE((lattice_compare::equivalent | lattice_compare::greater) == lattice_compare::equivalent);
    REQUIRE((lattice_compare::equivalent | lattice_compare::less) == lattice_compare::equivalent);
    REQUIRE((lattice_compare::equivalent | lattice_compare::unordered) == lattice_compare::equivalent);
    REQUIRE((lattice_compare::greater | lattice_compare::equivalent) == lattice_compare::equivalent);
    REQUIRE((lattice_compare::greater | lattice_compare::greater) == lattice_compare::greater);
    REQUIRE((lattice_compare::greater | lattice_compare::less) == lattice_compare::equivalent);
    REQUIRE((lattice_compare::greater | lattice_compare::unordered) == lattice_compare::greater);
    REQUIRE((lattice_compare::less | lattice_compare::equivalent) == lattice_compare::equivalent);
    REQUIRE((lattice_compare::less | lattice_compare::greater) == lattice_compare::equivalent);
    REQUIRE((lattice_compare::less | lattice_compare::less) == lattice_compare::less);
    REQUIRE((lattice_compare::less | lattice_compare::unordered) == lattice_compare::less);
    REQUIRE((lattice_compare::unordered | lattice_compare::equivalent) == lattice_compare::equivalent);
    REQUIRE((lattice_compare::unordered | lattice_compare::greater) == lattice_compare::greater);
    REQUIRE((lattice_compare::unordered | lattice_compare::less) == lattice_compare::less);
    REQUIRE((lattice_compare::unordered | lattice_compare::unordered) == lattice_compare::unordered);
}

TEST_CASE("boolean lattice") {
    lattice<bool> b;
    REQUIRE(b.top());
    REQUIRE(!b.bottom());
    REQUIRE(b.least_upper_bound(true, true));
    REQUIRE(b.least_upper_bound(true, false));
    REQUIRE(b.least_upper_bound(false, true));
    REQUIRE(!b.least_upper_bound(false, false));
    REQUIRE(b.greatest_lower_bound(true, true));
    REQUIRE(!b.greatest_lower_bound(true, false));
    REQUIRE(!b.greatest_lower_bound(false, true));
    REQUIRE(!b.greatest_lower_bound(false, false));
    REQUIRE(b.equal(true, true));
    REQUIRE(!b.equal(true, false));
    REQUIRE(!b.equal(false, true));
    REQUIRE(b.equal(false, false));
    REQUIRE(b.compare(true, true) == lattice_compare::equivalent);
    REQUIRE(b.compare(true, false) == lattice_compare::greater);
    REQUIRE(b.compare(false, true) == lattice_compare::less);
    REQUIRE(b.compare(false, false) == lattice_compare::equivalent);
    REQUIRE(b.hash(true) != b.hash(false));
}

} // namespace mtidd
