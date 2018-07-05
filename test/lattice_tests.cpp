#include "mtidd/mtidd.h"

//tell Catch to provide a main (only once pre cpp)
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

using namespace std;

namespace mtidd {

  TEST_CASE("lattice_compare") {
    REQUIRE(flip(Equal)     == Equal);
    REQUIRE(flip(Different) == Different);
    REQUIRE(flip(Smaller)   == Greater);
    REQUIRE(flip(Greater)   == Smaller);
    REQUIRE((Equal      &&  Equal)      == Equal);
    REQUIRE((Equal      &&  Greater)    == Greater);
    REQUIRE((Equal      &&  Smaller)    == Smaller);
    REQUIRE((Equal      &&  Different)  == Different);
    REQUIRE((Greater    &&  Equal)      == Greater);
    REQUIRE((Greater    &&  Greater)    == Greater);
    REQUIRE((Greater    &&  Smaller)    == Different);
    REQUIRE((Greater    &&  Different)  == Different);
    REQUIRE((Smaller    &&  Equal)      == Smaller);
    REQUIRE((Smaller    &&  Greater)    == Different);
    REQUIRE((Smaller    &&  Smaller)    == Smaller);
    REQUIRE((Smaller    &&  Different)  == Different);
    REQUIRE((Different  &&  Equal)      == Different);
    REQUIRE((Different  &&  Greater)    == Different);
    REQUIRE((Different  &&  Smaller)    == Different);
    REQUIRE((Different  &&  Different)  == Different);
    REQUIRE((Equal      ||  Equal)      == Equal);
    REQUIRE((Equal      ||  Greater)    == Equal);
    REQUIRE((Equal      ||  Smaller)    == Equal);
    REQUIRE((Equal      ||  Different)  == Equal);
    REQUIRE((Greater    ||  Equal)      == Equal);
    REQUIRE((Greater    ||  Greater)    == Greater);
    REQUIRE((Greater    ||  Smaller)    == Equal);
    REQUIRE((Greater    ||  Different)  == Greater);
    REQUIRE((Smaller    ||  Equal)      == Equal);
    REQUIRE((Smaller    ||  Greater)    == Equal);
    REQUIRE((Smaller    ||  Smaller)    == Smaller);
    REQUIRE((Smaller    ||  Different)  == Smaller);
    REQUIRE((Different  ||  Equal)      == Equal);
    REQUIRE((Different  ||  Greater)    == Greater);
    REQUIRE((Different  ||  Smaller)    == Smaller);
    REQUIRE((Different  ||  Different)  == Different);
  }

  TEST_CASE("boolean lattice") {
    lattice<bool> b;
    REQUIRE(b.top());
    REQUIRE(!b.bottom());
    REQUIRE( b.least_upper_bound(true, true));
    REQUIRE( b.least_upper_bound(true,false));
    REQUIRE( b.least_upper_bound(false,true));
    REQUIRE(!b.least_upper_bound(false,false));
    REQUIRE( b.greatest_lower_bound(true, true));
    REQUIRE(!b.greatest_lower_bound(true,false));
    REQUIRE(!b.greatest_lower_bound(false,true));
    REQUIRE(!b.greatest_lower_bound(false,false));
    REQUIRE( b.equal(true, true));
    REQUIRE(!b.equal(true,false));
    REQUIRE(!b.equal(false,true));
    REQUIRE( b.equal(false,false));
    REQUIRE(b.compare(true, true) == Equal);
    REQUIRE(b.compare(true,false) == Greater);
    REQUIRE(b.compare(false,true) == Smaller);
    REQUIRE(b.compare(false,false) == Equal);
    REQUIRE(b.hash(true) != b.hash(false));
  }

}
