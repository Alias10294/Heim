#include <concepts>
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
  int value{7};

  friend constexpr int &get(adl_map &m, int) noexcept { return m.value; }
  friend constexpr int const &get(adl_map const &m, int) noexcept { return m.value; }
};

struct both_map
{
  constexpr int get(int) const noexcept { return 11; }
  friend constexpr int get(both_map const &, int) noexcept { return 22; }
};
}

struct member_map
{
  int value{17};
  constexpr int &get(int) noexcept { return value; }
  constexpr int const &get(int) const noexcept { return value; }
};

struct qualified_map
{
  constexpr int get(int) & noexcept { return 1; }
  constexpr int get(int) const & noexcept { return 2; }
  constexpr int get(int) && noexcept { return 3; }
};

struct key_qualified_map
{
  constexpr int get(int &) const noexcept { return 1; }
  constexpr int get(int const &) const noexcept { return 2; }
  constexpr int get(int &&) const noexcept { return 3; }
};

struct throwing_map
{
  int get(int) const { throw std::runtime_error("get failure"); }
};

struct non_map
{
};

static_assert(std::same_as<decltype(heim::properties::get(std::declval<member_map &>(), 0)), int &>);
static_assert(
    std::same_as<decltype(heim::properties::get(std::declval<member_map const &>(), 0)), int const &>);
static_assert(std::same_as<decltype(heim::properties::get(std::declval<custom::adl_map &>(), 0)), int &>);
static_assert(
    std::same_as<decltype(heim::properties::get(std::declval<custom::adl_map const &>(), 0)), int const &>);
static_assert(noexcept(heim::properties::get(std::declval<member_map &>(), 0)));
static_assert(noexcept(heim::properties::get(std::declval<custom::adl_map &>(), 0)));
static_assert(!noexcept(heim::properties::get(std::declval<throwing_map &>(), 0)));
static_assert(!std::invocable<decltype(heim::properties::get), non_map &, int>);
static_assert(!std::invocable<decltype(heim::properties::get), member_map &, non_map>);
static_assert(heim::properties::get(qualified_map{}, 0) == 3);
static_assert(
    []
    {
      qualified_map m;
      return heim::properties::get(m, 0) == 1;
    }());
static_assert(
    []
    {
      qualified_map const m;
      return heim::properties::get(m, 0) == 2;
    }());
static_assert(
    []
    {
      key_qualified_map m;
      int k{};
      return heim::properties::get(m, k) == 1 && heim::properties::get(m, std::move(k)) == 3;
    }());
}

TEST_CASE("properties::get dispatches to member functions", "[property][primitives][get]")
{
  member_map map;
  auto &result = heim::properties::get(map, 3);
  REQUIRE(&result == &map.value);
  result = 42;
  REQUIRE(map.value == 42);
  member_map const &constant = map;
  REQUIRE(&heim::properties::get(constant, 3) == &map.value);
}

TEST_CASE("properties::get dispatches through ADL", "[property][primitives][get]")
{
  custom::adl_map map;
  auto &result = heim::properties::get(map, 4);
  REQUIRE(&result == &map.value);
  result = 19;
  REQUIRE(heim::properties::get(std::as_const(map), 4) == 19);
}

TEST_CASE("properties::get prefers a viable member over ADL", "[property][primitives][get]")
{
  custom::both_map map;
  REQUIRE(heim::properties::get(map, 1) == 11);
}

TEST_CASE("properties::get preserves map and key value categories", "[property][primitives][get]")
{
  qualified_map map;
  REQUIRE(heim::properties::get(map, 0) == 1);
  REQUIRE(heim::properties::get(std::as_const(map), 0) == 2);
  REQUIRE(heim::properties::get(std::move(map), 0) == 3);

  key_qualified_map key_map;
  int key{};
  int const constant_key{};
  REQUIRE(heim::properties::get(key_map, key) == 1);
  REQUIRE(heim::properties::get(key_map, constant_key) == 2);
  REQUIRE(heim::properties::get(key_map, 0) == 3);
}

TEST_CASE("properties::get propagates exceptions", "[property][primitives][get]")
{
  throwing_map map;
  REQUIRE_THROWS_AS(heim::properties::get(map, 0), std::runtime_error);
}

namespace
{
namespace fallback
{
struct map
{
  constexpr int get(char const *) const noexcept { return 2; }
  friend constexpr int get(map const &, int) noexcept { return 7; }
};

struct throwing_adl_map
{
  friend int get(throwing_adl_map, int) { throw std::runtime_error("ADL failure"); }
};
}
static_assert(noexcept(heim::properties::get(std::declval<fallback::map const &>(), 0)));
static_assert(!noexcept(heim::properties::get(std::declval<fallback::throwing_adl_map>(), 0)));
}

TEST_CASE("properties::get falls back to ADL if the member is not viable", "[property][primitives][get]")
{
  fallback::map map;
  REQUIRE(heim::properties::get(map, 0) == 7);
  REQUIRE(heim::properties::get(map, "x") == 2);
}

TEST_CASE("properties::get propagates exceptions from ADL customizations", "[property][primitives][get]")
{
  REQUIRE_THROWS_AS(heim::properties::get(fallback::throwing_adl_map{}, 0), std::runtime_error);
}
