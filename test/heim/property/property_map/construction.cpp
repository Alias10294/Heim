#include <catch2/catch_test_macros.hpp>
#include <heim/property.hpp>

#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>

namespace
{
struct getter
{
  int *value;
  constexpr int &operator()(int) const noexcept { return *value; }
};

struct setter
{
  int *value;
  constexpr void operator()(int, int x) const noexcept { *value = x; }
};

struct throwing_move
{
  throwing_move() = default;
  throwing_move(throwing_move const &) = default;
  throwing_move(throwing_move &&) noexcept(false) { }
  int operator()(int) const noexcept { return 1; }
};

using simple_map = heim::property_map<getter, setter>;
using throwing_map = heim::property_map<throwing_move, heim::no_property_setter_t>;
static_assert(std::is_nothrow_constructible_v<simple_map, getter, setter>);
static_assert(!std::is_nothrow_constructible_v<throwing_map, throwing_move, heim::no_property_setter_t>);
static_assert(std::copy_constructible<simple_map>);
static_assert(std::move_constructible<simple_map>);
static_assert(std::same_as<typename simple_map::getter_type, getter>);
static_assert(std::same_as<typename simple_map::setter_type, setter>);
static_assert(std::same_as<decltype(std::declval<simple_map &>().get(0)), int &>);
static_assert(noexcept(std::declval<simple_map &>().get(0)));
static_assert(noexcept(std::declval<simple_map &>().set(0, 1)));
}

TEST_CASE("property_map stores getter and setter function objects", "[property][property_map][construction]")
{
  int value{5};
  heim::property_map map{getter{&value}, setter{&value}};
  REQUIRE(&map.get(0) == &value);
  map.set(0, 13);
  REQUIRE(value == 13);
  heim::property_map copy{map};
  copy.set(2, 21);
  REQUIRE(map.get(2) == 21);
  heim::property_map moved{std::move(copy)};
  REQUIRE(moved.get(1) == 21);
}

TEST_CASE("property_map can own move-only function objects", "[property][property_map][construction]")
{
  auto getter_fn = [value = std::make_unique<int>(37)](int) { return *value; };
  auto setter_fn = [](int, int) { };
  heim::property_map map{std::move(getter_fn), setter_fn};
  REQUIRE(map.get(0) == 37);
  static_assert(!std::copy_constructible<decltype(map)>);
  static_assert(std::move_constructible<decltype(map)>);
}

TEST_CASE("property_map constructors work in constant evaluation", "[property][property_map][construction]")
{
  constexpr auto calculate = []
  {
    int value{7};
    heim::property_map map{getter{&value}, setter{&value}};
    map.set(0, 11);
    return map.get(0);
  };
  STATIC_REQUIRE(calculate() == 11);
}
