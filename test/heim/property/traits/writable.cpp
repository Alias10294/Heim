#include "support.hpp"

#include <catch2/catch_test_macros.hpp>
#include <concepts>

using namespace heim_test::property_traits;
namespace prop = heim::properties;

static_assert(prop::is_writable_map_for_v<read_write &, int, int>);
static_assert(prop::is_writable_map_for<read_write &, int, int>::value);
static_assert(prop::writable_map_for<read_write &, int, int>);
static_assert(prop::writable_map_for<write_only &, int, int>);
static_assert(prop::writable_map_for<adl::map &, int, int>);
static_assert(prop::writable_map_for<variadic_writer &, int, int, int>);
static_assert(!prop::writable_map_for<variadic_writer &, int, int>);
static_assert(!prop::writable_map_for<read_only &, int, int>);
static_assert(!prop::writable_map_for<empty &, int, int>);
static_assert(!prop::writable_map_for<read_write const &, int, int>);
static_assert(!prop::writable_map_for<key_sensitive &, double *, int>);
static_assert(prop::writable_map_for<read_reference_write_reference &, int, int &>);
static_assert(!prop::writable_map_for<read_reference_write_reference &, int, int>);

TEST_CASE("writable_map_for agrees with actual set invocations", "[property][traits][writable]")
{
  write_only map;
  prop::set(map, 1, 25);
  REQUIRE(map.value == 25);
  REQUIRE_FALSE((prop::readable_map_for<decltype(map) &, int>));
  REQUIRE((prop::writable_map_for<decltype(map) &, int, int>));
}
