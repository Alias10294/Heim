#include <concepts>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <heim/property.hpp>

namespace
{
struct mutable_getter
{
  int value{};
  int &operator()(int) noexcept { return value; }
};

struct mutable_setter
{
  int value{};
  void operator()(int, int v) noexcept { value = v; }
};

struct overloaded_getter
{
  int operator()(int) noexcept { return 1; }
  int operator()(int) const noexcept { return 2; }
};

struct overloaded_setter
{
  int *value;
  void operator()(int, int x) noexcept { *value = x; }
  void operator()(int, int x) const noexcept { *value = -x; }
};

struct multi_argument_setter
{
  int *value;
  void operator()(int k, int a, int b) const noexcept { *value = k + a + b; }
};

struct throwing_getter
{
  int operator()(int) const { throw std::runtime_error("getter"); }
};

struct throwing_setter
{
  void operator()(int, int) const { throw std::runtime_error("setter"); }
};

using mutable_map = heim::property_map<mutable_getter, mutable_setter>;
static_assert(heim::properties::readable_map_for<mutable_map &, int>);
static_assert(heim::properties::writable_map_for<mutable_map &, int, int>);
static_assert(!heim::properties::readable_map_for<mutable_map const &, int>);
static_assert(!heim::properties::writable_map_for<mutable_map const &, int, int>);
static_assert(!noexcept(std::declval<heim::property_map<throwing_getter, throwing_setter> &>().get(0)));
static_assert(!noexcept(std::declval<heim::property_map<throwing_getter, throwing_setter> &>().set(0, 0)));
}

TEST_CASE("property_map conditionally exposes const-qualified access", "[property][property_map][access]")
{
  heim::property_map map{overloaded_getter{}, heim::no_property_setter};
  REQUIRE(map.get(0) == 1);
  REQUIRE(std::as_const(map).get(0) == 2);
  REQUIRE(heim::properties::get(map, 0) == 1);
  REQUIRE(heim::properties::get(std::as_const(map), 0) == 2);
}

TEST_CASE("property_map selects mutable and const setter overloads", "[property][property_map][access]")
{
  int value{};
  heim::property_map map{heim::no_property_getter, overloaded_setter{&value}};
  map.set(0, 13);
  REQUIRE(value == 13);
  std::as_const(map).set(0, 17);
  REQUIRE(value == -17);
}

TEST_CASE("property_map supports multi-argument setters", "[property][property_map][access]")
{
  int value{};
  heim::property_map map{heim::no_property_getter, multi_argument_setter{&value}};
  heim::properties::set(map, 2, 3, 4);
  REQUIRE(value == 9);
}

TEST_CASE("property_map propagates getter and setter exceptions", "[property][property_map][access]")
{
  heim::property_map map{throwing_getter{}, throwing_setter{}};
  REQUIRE_THROWS_AS(map.get(0), std::runtime_error);
  REQUIRE_THROWS_AS(map.set(0, 1), std::runtime_error);
}
