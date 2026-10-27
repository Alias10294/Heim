#include <catch2/catch_test_macros.hpp>
#include <heim/property.hpp>
#include <unordered_map>

TEST_CASE("associative property map supports reading and writing", "[property]")
{
  std::unordered_map<int, int> values{{0, 100}, {1, 80}};
  auto pm = heim::make_property_map(values);

  REQUIRE(heim::properties::get(pm, 1) == 80);
  heim::properties::set(pm, 1, 42);
  REQUIRE(values.at(1) == 42);
  REQUIRE(heim::properties::get(pm, 0) == 100);
}
