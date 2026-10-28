#include "support.hpp"

#include <catch2/catch_test_macros.hpp>
#include <concepts>

using namespace heim_test::property_traits;
namespace prop = heim::properties;

static_assert(prop::is_reflective_map_for_v<read_write &, int>);
static_assert(prop::is_reflective_map_for<read_write &, int>::value);
static_assert(prop::reflective_map_for<read_write &, int>);
static_assert(prop::reflective_map_for<read_reference_write_reference &, int>);
static_assert(prop::reflective_map_for<adl::map &, int>);
static_assert(!prop::reflective_map_for<read_only const &, int>);
static_assert(!prop::reflective_map_for<read_value_write_reference &, int>);
// A non-readable map must also be rejected without a hard substitution error.
static_assert(!prop::reflective_map_for<write_only &, int>);
static_assert(!prop::reflective_map_for<empty &, int>);

TEST_CASE("reflective_map_for accepts writing back read values", "[property][traits][reflective]")
{
  read_reference_write_reference map;
  int original{7};
  prop::set(map, 0, original);
  REQUIRE(prop::get(map, 0) == 7);
  REQUIRE((prop::reflective_map_for<decltype(map) &, int>));
}
