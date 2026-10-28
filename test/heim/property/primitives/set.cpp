#include <concepts>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <heim/property.hpp>

namespace
{
namespace custom
{
struct adl_map
{
  int value{};
  friend constexpr void set(adl_map &m, int, int v) noexcept { m.value = v; }
};

struct both_map
{
  int value{};
  constexpr void set(int, int v) noexcept { value = v; }
  friend constexpr void set(both_map &m, int, int v) noexcept { m.value = -v; }
};
}

struct member_map
{
  int value{};
  constexpr void set(int, int v) noexcept { value = v; }
};

struct const_setter
{
  int *value;
  constexpr void set(int, int v) const noexcept { *value = v; }
};

struct forwarding_setter
{
  int selected{};
  constexpr void set(int &, int &) noexcept { selected = 1; }
  constexpr void set(int const &, int const &) noexcept { selected = 2; }
  constexpr void set(int &&, int &&) noexcept { selected = 3; }
};

struct multi_setter
{
  int sum{};
  constexpr void set(int key, int x, int y, int z) noexcept { sum = key + x + y + z; }
};

struct moving_setter
{
  std::unique_ptr<int> value;
  void set(int, std::unique_ptr<int> v) noexcept { value = std::move(v); }
};

struct throwing_setter
{
  void set(int, int) { throw std::runtime_error("set failure"); }
};

struct non_map
{
};

static_assert(std::same_as<decltype(heim::properties::set(std::declval<member_map &>(), 0, 1)), void>);
static_assert(noexcept(heim::properties::set(std::declval<member_map &>(), 0, 1)));
static_assert(noexcept(heim::properties::set(std::declval<custom::adl_map &>(), 0, 1)));
static_assert(!noexcept(heim::properties::set(std::declval<throwing_setter &>(), 0, 1)));
static_assert(!std::invocable<decltype(heim::properties::set), non_map &, int, int>);
static_assert(!std::invocable<decltype(heim::properties::set), member_map const &, int, int>);
static_assert(!std::invocable<decltype(heim::properties::set), member_map &, int, non_map>);
static_assert(std::invocable<decltype(heim::properties::set), const_setter const &, int, int>);
static_assert(
    []
    {
      member_map m;
      heim::properties::set(m, 0, 8);
      return m.value == 8;
    }());
}

TEST_CASE("properties::set dispatches to member functions", "[property][primitives][set]")
{
  member_map map;
  heim::properties::set(map, 2, 42);
  REQUIRE(map.value == 42);
}

TEST_CASE("properties::set dispatches through ADL", "[property][primitives][set]")
{
  custom::adl_map map;
  heim::properties::set(map, 2, 17);
  REQUIRE(map.value == 17);
}

TEST_CASE("properties::set prefers a viable member over ADL", "[property][primitives][set]")
{
  custom::both_map map;
  heim::properties::set(map, 0, 19);
  REQUIRE(map.value == 19);
}

TEST_CASE("properties::set respects const-qualified setters", "[property][primitives][set]")
{
  int value{};
  const_setter const map{&value};
  heim::properties::set(map, 0, 63);
  REQUIRE(value == 63);
}

TEST_CASE("properties::set perfectly forwards keys and values", "[property][primitives][set]")
{
  forwarding_setter map;
  int key{}, value{};
  int const const_key{}, const_value{};
  heim::properties::set(map, key, value);
  REQUIRE(map.selected == 1);
  heim::properties::set(map, const_key, const_value);
  REQUIRE(map.selected == 2);
  heim::properties::set(map, 0, 0);
  REQUIRE(map.selected == 3);
}

TEST_CASE("properties::set forwards variadic arguments", "[property][primitives][set]")
{
  multi_setter map;
  heim::properties::set(map, 2, 3, 5, 7);
  REQUIRE(map.sum == 17);
}

TEST_CASE("properties::set supports move-only arguments", "[property][primitives][set]")
{
  moving_setter map;
  auto value = std::make_unique<int>(99);
  heim::properties::set(map, 0, std::move(value));
  REQUIRE_FALSE(value);
  REQUIRE(map.value);
  REQUIRE(*map.value == 99);
}

TEST_CASE("properties::set propagates exceptions", "[property][primitives][set]")
{
  throwing_setter map;
  REQUIRE_THROWS_AS(heim::properties::set(map, 0, 9), std::runtime_error);
}

namespace
{
namespace fallback
{
struct map
{
  int value{};
  constexpr void set(char const *, int v) noexcept { value = v + 1; }
  friend constexpr void set(map &m, int, int v) noexcept { m.value = v; }
};

struct throwing_adl_map
{
  friend void set(throwing_adl_map &, int, int) { throw std::runtime_error("ADL failure"); }
};
}
static_assert(noexcept(heim::properties::set(std::declval<fallback::map &>(), 0, 1)));
static_assert(!noexcept(heim::properties::set(std::declval<fallback::throwing_adl_map &>(), 0, 1)));
}

TEST_CASE("properties::set falls back to ADL if the member is not viable", "[property][primitives][set]")
{
  fallback::map map;
  heim::properties::set(map, 0, 7);
  REQUIRE(map.value == 7);
  heim::properties::set(map, "x", 7);
  REQUIRE(map.value == 8);
}

TEST_CASE("properties::set propagates exceptions from ADL customizations", "[property][primitives][set]")
{
  fallback::throwing_adl_map map;
  REQUIRE_THROWS_AS(heim::properties::set(map, 0, 1), std::runtime_error);
}
