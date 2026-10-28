#include <catch2/catch_test_macros.hpp>

#include <heim/property/property_map.hpp>

TEST_CASE("property_map.hpp can be included independently", "[property][headers]")
{
  int value{9};
  auto map = heim::make_property_map(heim::pointer_arg);
  REQUIRE(heim::properties::get(map, &value) == 9);
  heim::properties::set(map, &value, 18);
  REQUIRE(value == 18);
}
