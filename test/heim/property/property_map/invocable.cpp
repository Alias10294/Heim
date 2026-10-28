#include <concepts>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <heim/property.hpp>

namespace
{
namespace prop = heim::properties;

int double_value(int x) noexcept
{
  return x * 2;
}

struct ref_getter
{
  int *storage;
  int &operator()(int) const noexcept { return *storage; }
};

struct counter_getter
{
  int *calls;
  int operator()(int v) const
  {
    ++*calls;
    return v + 1;
  }
};

struct throwing_getter
{
  int operator()(int) const { throw std::runtime_error("invocable"); }
};
}

TEST_CASE("invocable_arg creates a map from a callable", "[property][property_map][invocable]")
{
  auto map = heim::make_property_map(heim::invocable_arg, [](int x) noexcept { return x * x; });
  REQUIRE(prop::get(map, 3) == 9);
  REQUIRE(prop::get(map, -4) == 16);
  static_assert(!prop::writable_map_for<decltype(map) &, int, int>);
  static_assert(noexcept(prop::get(map, 3)));
}

TEST_CASE(
    "invocable_arg can create writable maps from reference-returning callables",
    "[property][property_map][invocable]")
{
  int storage{11};
  auto map = heim::make_property_map(heim::invocable_arg, ref_getter{&storage});
  REQUIRE(&prop::get(map, 123) == &storage);
  prop::set(map, 456, 78);
  REQUIRE(storage == 78);
}

TEST_CASE("invocable_arg accepts std::identity", "[property][property_map][invocable]")
{
  auto map = heim::make_property_map(heim::invocable_arg, std::identity{});
  int key{12};
  REQUIRE(&prop::get(map, key) == &key);
  prop::set(map, key, 55);
  REQUIRE(key == 55);
}

TEST_CASE(
    "callable adapters copy and invoke their captured function object",
    "[property][property_map][invocable]")
{
  int calls{};
  counter_getter fn{&calls};
  auto map = heim::make_property_map(heim::invocable_arg, fn);
  REQUIRE(prop::get(map, 1) == 2);
  REQUIRE(prop::get(map, 3) == 4);
  REQUIRE(calls == 2);
}

TEST_CASE(
    "function pointers have an automatic make_property_map overload",
    "[property][property_map][invocable]")
{
  auto tagged = heim::make_property_map(heim::invocable_arg, &double_value);
  auto automatic = heim::make_property_map(double_value);
  REQUIRE(prop::get(tagged, 6) == 12);
  REQUIRE(prop::get(automatic, 9) == 18);
}

TEST_CASE("invocable adapter propagates exceptions", "[property][property_map][invocable]")
{
  auto map = heim::make_property_map(heim::invocable_arg, throwing_getter{});
  REQUIRE_THROWS_AS(prop::get(map, 0), std::runtime_error);
}
