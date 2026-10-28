#include <heim/property/primitives.hpp>

#include <catch2/catch_test_macros.hpp>
#include <concepts>
#include <type_traits>

namespace
{
struct simple_map
{
  int value{};
  constexpr int &get(int) noexcept { return value; }
  constexpr void set(int, int x) noexcept { value = x; }
};
static_assert(std::invocable<decltype(heim::properties::get), simple_map &, int>);
static_assert(std::invocable<decltype(heim::properties::set), simple_map &, int, int>);
}

TEST_CASE("primitives.hpp can be included independently", "[property][headers]")
{
  simple_map map;
  heim::properties::set(map, 0, 47);
  REQUIRE(heim::properties::get(map, 0) == 47);
}
