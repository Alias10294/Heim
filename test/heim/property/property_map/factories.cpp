#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <heim/property.hpp>

namespace
{
namespace prop = heim::properties;
using pointer_map = decltype(heim::make_property_map(heim::pointer_arg));
using readable_pointer_map = decltype(heim::make_readable_property_map(heim::pointer_arg));
using writable_pointer_map = decltype(heim::make_writable_property_map(heim::pointer_arg));

static_assert(prop::map_for<pointer_map &, int *, int>);
static_assert(prop::readable_map_for<readable_pointer_map &, int *>);
static_assert(!prop::writable_map_for<readable_pointer_map &, int *, int>);
static_assert(!prop::readable_map_for<writable_pointer_map &, int *>);
static_assert(prop::writable_map_for<writable_pointer_map &, int *, int>);
static_assert(std::same_as<typename readable_pointer_map::setter_type, heim::no_property_setter_t>);
static_assert(std::same_as<typename writable_pointer_map::getter_type, heim::no_property_getter_t>);
}

TEST_CASE("make_property_map creates readable and writable maps", "[property][property_map][factories]")
{
  int value{4};
  auto map = heim::make_property_map(heim::pointer_arg);
  REQUIRE(&prop::get(map, &value) == &value);
  prop::set(map, &value, 31);
  REQUIRE(value == 31);
}

TEST_CASE("make_readable_property_map suppresses write access", "[property][property_map][factories]")
{
  int value{6};
  auto map = heim::make_readable_property_map(heim::pointer_arg);
  REQUIRE(prop::get(map, &value) == 6);
  REQUIRE_FALSE((prop::writable_map_for<decltype(map) &, int *, int>));
}

TEST_CASE("make_writable_property_map suppresses read access", "[property][property_map][factories]")
{
  int value{8};
  auto map = heim::make_writable_property_map(heim::pointer_arg);
  prop::set(map, &value, 23);
  REQUIRE(value == 23);
  REQUIRE_FALSE((prop::readable_map_for<decltype(map) &, int *>));
}

TEST_CASE("readable and writable factories work with adapted ranges", "[property][property_map][factories]")
{
  int values[]{1, 2, 3};
  auto read = heim::make_readable_property_map(values);
  auto write = heim::make_writable_property_map(values);
  REQUIRE(prop::get(read, 1) == 2);
  prop::set(write, 1, 9);
  REQUIRE(values[1] == 9);
  REQUIRE_FALSE((prop::writable_map_for<decltype(read) &, int, int>));
  REQUIRE_FALSE((prop::readable_map_for<decltype(write) &, int>));
}

TEST_CASE(
    "const property_map object can still write through a mutable referenced resource",
    "[property][property_map][factories]")
{
  int values[]{3, 4};
  auto const map = heim::make_property_map(values);
  static_assert(prop::writable_map_for<decltype(map) &, int, int>);
  prop::set(map, 0, 19);
  REQUIRE(values[0] == 19);
}
