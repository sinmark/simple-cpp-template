#include <catch2/catch_all.hpp>

#include "project/tmp.hpp"

TEST_CASE("Test tmp lib", "[tmp]")
{
  REQUIRE(tmp::add(5, 5) == 10);
}