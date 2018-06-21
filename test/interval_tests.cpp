#include "mtidd/mtidd.h"

//tell Catch to provide a main (only once pre cpp)
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

using namespace std;

namespace mtidd {

  TEST_CASE("boundary complement") {
    REQUIRE(complement(Open) == Closed);
    REQUIRE(complement(Closed) == Open);
  }

  TEST_CASE("half_interval"){

    half_interval hi = make_tuple(0.0, Closed);
    REQUIRE( contains(hi, -1.0));
    REQUIRE( contains(hi,  0.0));
    REQUIRE(!contains(hi,  1.0));
    REQUIRE( contains(hi, -numeric_limits<double>::infinity()));
    REQUIRE(!contains(hi, numeric_limits<double>::infinity()));

    half_interval hio = make_tuple(0.0, Open);
    REQUIRE( contains(hio, -1.0));
    REQUIRE(!contains(hio,  0.0));
    REQUIRE(!contains(hio,  1.0));

    REQUIRE(min(hio,  hi) == hio);
    REQUIRE(min(hi,  hio) == hio);
    REQUIRE(min(lower_sentinel,  hi) == lower_sentinel);
    REQUIRE(min(lower_sentinel,  hio) == lower_sentinel);
    REQUIRE(min(upper_sentinel,  hi) == hi);
    REQUIRE(min(upper_sentinel,  hio) == hio);

    REQUIRE(hio <= hi);
    REQUIRE(hio < hi);
    REQUIRE(!(hi <= hio));
    REQUIRE(!(hi < hio));
    REQUIRE(lower_sentinel <= hi);
    REQUIRE(lower_sentinel <= hio);
    REQUIRE(lower_sentinel < hi);
    REQUIRE(lower_sentinel < hio);
    REQUIRE(!(upper_sentinel <= hi));
    REQUIRE(!(upper_sentinel <= hio));
    REQUIRE(!(upper_sentinel < hi));
    REQUIRE(!(upper_sentinel < hio));
    REQUIRE(hi <= upper_sentinel);
    REQUIRE(hi < upper_sentinel);
    REQUIRE(hio <= upper_sentinel);
    REQUIRE(hio < upper_sentinel);
    REQUIRE(!(hi <= lower_sentinel));
    REQUIRE(!(hi < lower_sentinel));
    REQUIRE(!(hio <= lower_sentinel));
    REQUIRE(!(hio < lower_sentinel));
  }

  TEST_CASE("interval"){
    interval i1 = make_tuple(-10, Open, 10, Closed);
    REQUIRE(!contains(i1,-20) );
    REQUIRE(!contains(i1,-10) );
    REQUIRE( contains(i1,  0) );
    REQUIRE( contains(i1, 10) );
    REQUIRE(!contains(i1, 20) );
    REQUIRE(!is_empty(i1));
    REQUIRE(!is_singleton(i1));

    interval i2 = make_tuple(0, Closed, 0, Closed);
    REQUIRE(!contains(i2,-20) );
    REQUIRE(!contains(i2,-10) );
    REQUIRE( contains(i2,  0) );
    REQUIRE(!contains(i2, 10) );
    REQUIRE(!contains(i2, 20) );
    REQUIRE(!is_empty(i2));
    REQUIRE( is_singleton(i2));
    
    interval i3 = make_tuple(0, Open, 0, Open);
    REQUIRE(!contains(i3,-20) );
    REQUIRE(!contains(i3,-10) );
    REQUIRE(!contains(i3,  0) );
    REQUIRE(!contains(i3, 10) );
    REQUIRE(!contains(i3, 20) );
    REQUIRE( is_empty(i3));
    REQUIRE(!is_singleton(i3));

    REQUIRE( overlap(i1, i2));
    REQUIRE( overlap(i2, i1));
    REQUIRE(!overlap(i1, i3));
    REQUIRE(!overlap(i3, i1));
    REQUIRE(!overlap(i2, i3));
    REQUIRE(!overlap(i3, i2));
    
    REQUIRE( covers(i1, i2));
    REQUIRE(!covers(i2, i1));
    REQUIRE(!covers(i1, i3));
    REQUIRE(!covers(i3, i1));
    REQUIRE(!covers(i2, i3));
    REQUIRE(!covers(i3, i2));
  }

}
