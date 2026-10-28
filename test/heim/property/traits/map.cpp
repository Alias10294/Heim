#include "support.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace heim_test::property_traits;
namespace prop = heim::properties;

static_assert(prop::is_map_for_v<read_write &, int, int>);
static_assert(prop::is_map_for<read_write &, int, int>::value);
static_assert(prop::map_for<read_write &, int, int>);
static_assert(prop::map_for<adl::map &, int, int>);
static_assert(!prop::map_for<read_only &, int, int>);
static_assert(!prop::map_for<write_only &, int, int>);
static_assert(!prop::map_for<empty &, int, int>);
static_assert(!prop::map_for<read_write const &, int, int>);
static_assert(!prop::map_for<key_sensitive &, double *, int>);
static_assert(prop::map_for<read_reference_write_reference &, int, int &>);
static_assert(!prop::map_for<read_reference_write_reference &, int, int>);

TEST_CASE("map_for requires both reading and writing for the same key", "[property][traits][map]")
{
  read_write map;
  prop::set(map, 3, 14);
  REQUIRE(prop::get(map, 3) == 14);
  REQUIRE((prop::map_for<decltype(map) &, int, int>));
}
