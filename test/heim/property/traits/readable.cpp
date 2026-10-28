#include "support.hpp"

#include <concepts>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>

using namespace heim_test::property_traits;
namespace prop = heim::properties;

static_assert(prop::is_readable_map_for_v<read_write &, int>);
static_assert(prop::is_readable_map_for<read_write &, int>::value);
static_assert(prop::readable_map_for<read_write &, int>);
static_assert(prop::readable_map_for<read_only const &, int>);
static_assert(prop::readable_map_for<adl::map &, int>);
static_assert(!prop::readable_map_for<write_only &, int>);
static_assert(!prop::readable_map_for<empty &, int>);
static_assert(!prop::readable_map_for<key_sensitive const &, double *>);
static_assert(!prop::readable_map_for<read_write const &, int>);
static_assert(std::same_as<prop::readable_map_result_t<read_write &, int>, int &>);
static_assert(std::same_as<prop::readable_map_result_t<read_only const &, int>, int const &>);
static_assert(std::same_as<prop::readable_map_result_t<key_sensitive const &, int>, int>);
static_assert(std::same_as<prop::readable_map_result_t<adl::map &, int>, int &>);
static_assert(std::same_as<typename prop::readable_map_result<read_write &, int>::type, int &>);

TEST_CASE("readable_map_for agrees with actual get invocations", "[property][traits][readable]")
{
  read_write m;
  prop::get(m, 0) = 37;
  REQUIRE(prop::readable_map_for<decltype(m) &, int>);
  REQUIRE(prop::get(m, 0) == 37);
  REQUIRE_FALSE((prop::readable_map_for<decltype(m) const &, int>));
}
