#include <catch2/catch_test_macros.hpp>
#include <heim/property.hpp>
#include <array>
#include <deque>
#include <map>
#include <span>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace {
struct entity { int hp; int armor; };
}

TEST_CASE("property CPO reads and writes ordered and unordered maps", "[property]")
{
  std::map<unsigned, int> ordered{{2, 10}};
  std::unordered_map<unsigned, int> unordered{{4, 12}};
  auto pm1 = heim::make_property_map(ordered);
  auto pm2 = heim::make_property_map(unordered);
  REQUIRE(heim::properties::get(pm1, 2u) == 10);
  REQUIRE(heim::properties::get(pm2, 4u) == 12);
  heim::properties::set(pm1, 2u, 20);
  heim::properties::set(pm2, 9u, 99);
  REQUIRE(ordered.at(2) == 20);
  REQUIRE(unordered.at(9) == 99);
}

TEST_CASE("property CPO accepts const associative container as readable map", "[property]")
{
  const std::unordered_map<int, int> data{{1, 11}, {3, 33}};
  auto pm = heim::make_readable_property_map(data);
  REQUIRE(heim::properties::get(pm, 3) == 33);
  REQUIRE(heim::properties::get(pm, 1) == 11);
}

TEST_CASE("vector, array, deque and span maps use random access indices", "[property]")
{
  std::vector<int> vec{10, 20, 30};
  std::array<int, 3> arr{40, 50, 60};
  std::deque<int> deq{70, 80, 90};
  std::span<int> view{vec};
  auto a = heim::make_property_map(vec);
  auto b = heim::make_property_map(arr);
  auto c = heim::make_property_map(deq);
  auto d = heim::make_property_map(view);
  REQUIRE(heim::properties::get(a, 1) == 20);
  REQUIRE(heim::properties::get(b, 2) == 60);
  REQUIRE(heim::properties::get(c, 0) == 70);
  REQUIRE(heim::properties::get(d, 2) == 30);
  heim::properties::set(a, 0, 11);
  heim::properties::set(b, 1, 51);
  heim::properties::set(c, 2, 91);
  heim::properties::set(d, 2, 31);
  REQUIRE((vec == std::vector<int>{11, 20, 31}));
  REQUIRE((arr == std::array<int, 3>{40, 51, 60}));
  REQUIRE(deq[2] == 91);
}

TEST_CASE("index property map adapts non-contiguous logical keys", "[property]")
{
  std::vector<int> values{100, 200, 300};
  std::unordered_map<unsigned, std::ptrdiff_t> positions{{9u, 0}, {42u, 2}};
  auto pm = heim::make_property_map(values, heim::make_readable_property_map(positions));
  REQUIRE(heim::properties::get(pm, 42u) == 300);
  heim::properties::set(pm, 9u, -100);
  REQUIRE(values[0] == -100);
}

TEST_CASE("member property map reads and modifies aggregates", "[property]")
{
  entity e{100, 30};
  auto pm = heim::make_property_map(heim::member_arg, &entity::hp);
  REQUIRE(heim::properties::get(pm, e) == 100);
  heim::properties::set(pm, e, 65);
  REQUIRE(e.hp == 65);
  REQUIRE(e.armor == 30);
}

TEST_CASE("composed map traverses container then member", "[property]")
{
  std::vector<entity> entities{{100, 5}, {80, 10}, {60, 15}};
  auto pm = heim::make_property_map(
      heim::composable_arg,
      heim::make_property_map(heim::member_arg, &entity::armor),
      heim::make_property_map(entities));
  REQUIRE(heim::properties::get(pm, 1) == 10);
  heim::properties::set(pm, 2, 75);
  REQUIRE(entities[2].armor == 75);
  REQUIRE(entities[2].hp == 60);
}

TEST_CASE("pointer property map reads and writes through pointer keys", "[property]")
{
  int x = 7;
  int y = 11;
  auto pm = heim::make_property_map(heim::pointer_arg);
  REQUIRE(heim::properties::get(pm, &x) == 7);
  REQUIRE(heim::properties::get(pm, &y) == 11);
  heim::properties::set(pm, &x, 42);
  REQUIRE(x == 42);
}

TEST_CASE("identity, constant and callable maps return expected values", "[property]")
{
  auto identity = heim::make_readable_property_map(heim::identity_arg);
  auto constant = heim::make_readable_property_map(heim::constant_arg, 17);
  auto callable = heim::make_readable_property_map(heim::invocable_arg, [](int v) { return v * v; });
  int x = 5;
  REQUIRE(heim::properties::get(identity, x) == 5);
  REQUIRE(heim::properties::get(constant, x) == 17);
  REQUIRE(heim::properties::get(constant, 99) == 17);
  REQUIRE(heim::properties::get(callable, x) == 25);
}
