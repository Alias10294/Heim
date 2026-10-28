#include <catch2/catch_test_macros.hpp>
#include <heim/property.hpp>

#include <concepts>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
namespace prop = heim::properties;

struct index_failure { };
struct read_failure { };
struct write_failure { };

struct entity
{
  int hp{100};
  int armor{5};
};

namespace external
{
struct index_map
{
  friend constexpr std::size_t get(index_map, int key) noexcept
  { return static_cast<std::size_t>(key + 1); }
};
}
}

TEST_CASE("composable_arg composes range and member property maps", "[property][property_map][composition]")
{
  std::vector<entity> entities{{10, 1}, {20, 2}, {30, 3}};
  auto member = heim::make_property_map(heim::member_arg, &entity::hp);
  auto range = heim::make_property_map(entities);
  auto composed = heim::make_property_map(heim::composable_arg, member, range);
  REQUIRE(&prop::get(composed, 1) == &entities[1].hp);
  prop::set(composed, 1, 77);
  REQUIRE(entities[1].hp == 77);
  REQUIRE(entities[1].armor == 2);
  static_assert(prop::map_for<decltype(composed) &, int, int>);
}

TEST_CASE("composable_arg composes associative maps and pointer maps", "[property][property_map][composition]")
{
  std::map<int, std::unique_ptr<std::string>> objects;
  objects.emplace(1, std::make_unique<std::string>("first"));
  auto pointer = heim::make_property_map(heim::pointer_arg);
  auto associative = heim::make_property_map(objects);
  auto composed = heim::make_property_map(heim::composable_arg, pointer, associative);
  REQUIRE(&prop::get(composed, 1) == objects.at(1).get());
  prop::set(composed, 1, std::string{"second"});
  REQUIRE(*objects.at(1) == "second");
}

TEST_CASE("nested composition evaluates maps in the documented order", "[property][property_map][composition]")
{
  std::vector<entity> entities{{10, 1}, {20, 2}, {30, 3}};
  std::unordered_map<int, int> indices{{9, 1}, {7, 2}};
  auto idx = heim::make_property_map(indices);
  auto range = heim::make_property_map(entities);
  auto member = heim::make_property_map(heim::member_arg, &entity::hp);
  auto from_key = heim::make_property_map(heim::composable_arg, range, idx);
  auto composed = heim::make_property_map(heim::composable_arg, member, from_key);
  REQUIRE(prop::get(composed, 9) == 20);
  REQUIRE(prop::get(composed, 7) == 30);
  prop::set(composed, 7, 123);
  REQUIRE(entities[2].hp == 123);
}

TEST_CASE("composition accepts externally customized property maps", "[property][property_map][composition]")
{
  std::vector<int> values{3, 5, 7, 9};
  auto range = heim::make_property_map(values);
  auto composed = heim::make_property_map(heim::composable_arg, range, external::index_map{});
  REQUIRE(prop::get(composed, 0) == 5);
  REQUIRE(prop::get(composed, 2) == 9);
  prop::set(composed, 1, 17);
  REQUIRE(values[2] == 17);
}

TEST_CASE("composition of readable-only maps stays readable-only", "[property][property_map][composition]")
{
  std::vector<entity> entities{{10, 1}};
  auto member = heim::make_readable_property_map(heim::member_arg, &entity::hp);
  auto range = heim::make_property_map(entities);
  auto composed = heim::make_property_map(heim::composable_arg, member, range);
  REQUIRE(prop::get(composed, 0) == 10);
  static_assert(prop::readable_map_for<decltype(composed) &, int>);
  static_assert(!prop::writable_map_for<decltype(composed) &, int, int>);
}

TEST_CASE("composition propagates failures from the index map", "[property][property_map][composition]")
{
  int reads{};
  int writes{};
  auto outer = heim::property_map{
      [&](int) { ++reads; return 0; },
      [&](int, int) { ++writes; }};
  auto inner = heim::property_map{
      [](int) -> int { throw index_failure{}; },
      heim::no_property_setter};
  auto composed = heim::make_property_map(heim::composable_arg, outer, inner);

  REQUIRE_THROWS_AS(prop::get(composed, 3), index_failure);
  REQUIRE_THROWS_AS(prop::set(composed, 3, 9), index_failure);
  REQUIRE(reads == 0);
  REQUIRE(writes == 0);
}

TEST_CASE("composition propagates failures from the outer map", "[property][property_map][composition]")
{
  int reads{};
  int writes{};
  auto outer = heim::property_map{
      [&](int) -> int { ++reads; throw read_failure{}; },
      [&](int, int) { ++writes; throw write_failure{}; }};
  auto inner = heim::make_property_map(heim::invocable_arg, [](int k) noexcept { return k + 1; });
  auto composed = heim::make_property_map(heim::composable_arg, outer, inner);

  REQUIRE_THROWS_AS(prop::get(composed, 3), read_failure);
  REQUIRE_THROWS_AS(prop::set(composed, 3, 9), write_failure);
  REQUIRE(reads == 1);
  REQUIRE(writes == 1);
}

TEST_CASE("composition propagates noexcept from both maps", "[property][property_map][composition]")
{
  int received_key{-1};
  int received_value{-1};
  auto inner = heim::make_property_map(heim::invocable_arg,
      [](int key) noexcept { return key + 1; });
  auto outer = heim::property_map{
      [](int key) noexcept { return key * 2; },
      [&](int key, int value) noexcept
      {
        received_key = key;
        received_value = value;
      }};
  auto composed = heim::make_property_map(heim::composable_arg, outer, inner);
  static_assert(noexcept(prop::get(composed, 3)));
  static_assert(noexcept(prop::set(composed, 3, 10)));
  REQUIRE(prop::get(composed, 3) == 8);
  prop::set(composed, 3, 10);
  REQUIRE(received_key == 4);
  REQUIRE(received_value == 10);

  auto may_throw_on_read = heim::property_map{
      [](int key) { return key; },
      [](int, int) noexcept { }};
  auto read_composed = heim::make_property_map(heim::composable_arg, may_throw_on_read, inner);
  static_assert(!noexcept(prop::get(read_composed, 3)));
  static_assert(noexcept(prop::set(read_composed, 3, 10)));

  auto may_throw_on_write = heim::property_map{
      [](int key) noexcept { return key; },
      [](int, int) { }};
  auto write_composed = heim::make_property_map(heim::composable_arg, may_throw_on_write, inner);
  static_assert(noexcept(prop::get(write_composed, 3)));
  static_assert(!noexcept(prop::set(write_composed, 3, 10)));

  auto may_throw_on_index = heim::make_property_map(heim::invocable_arg,
      [](int key) { return key + 1; });
  auto index_composed = heim::make_property_map(heim::composable_arg, outer, may_throw_on_index);
  static_assert(!noexcept(prop::get(index_composed, 3)));
  static_assert(!noexcept(prop::set(index_composed, 3, 10)));
}
