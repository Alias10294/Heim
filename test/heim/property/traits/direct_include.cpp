#include <heim/property/traits.hpp>

#include <catch2/catch_test_macros.hpp>
#include <type_traits>

namespace
{
struct simple_map
{
  constexpr int get(int) const noexcept { return 4; }
};
static_assert(heim::properties::readable_map_for<simple_map const &, int>);
static_assert(!heim::properties::writable_map_for<simple_map const &, int, int>);
static_assert(std::is_same_v<heim::properties::readable_map_result_t<simple_map const &, int>, int>);
}

TEST_CASE("traits.hpp can be included independently", "[property][headers]")
{
  simple_map map;
  REQUIRE(heim::properties::get(map, 0) == 4);
}
