#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"

#include "index_map.hpp"

TEST_CASE("heim::index_map (DEBUG)")
{
  heim::index_map<std::size_t, int> im;
}
