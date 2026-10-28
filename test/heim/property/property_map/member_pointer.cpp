#include <catch2/catch_test_macros.hpp>
#include <heim/property.hpp>

#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace
{
namespace prop = heim::properties;

struct entity
{
  int hp{100};
  int mana{50};

  int &health() noexcept { return hp; }
  int const &health() const noexcept { return hp; }
  int strength() const noexcept { return hp + mana; }
};

using member_map = decltype(heim::make_property_map(heim::member_arg, &entity::hp));
using function_map = decltype(heim::make_property_map(heim::member_arg, static_cast<int &(entity::*)()>(&entity::health)));
static_assert(prop::map_for<member_map &, entity &, int>);
static_assert(prop::readable_map_for<member_map &, entity const &>);
static_assert(!prop::writable_map_for<member_map &, entity const &, int>);
static_assert(prop::map_for<function_map &, entity &, int>);
static_assert(std::same_as<decltype(prop::get(std::declval<member_map &>(), std::declval<entity &>())), int &>);
static_assert(std::same_as<decltype(prop::get(std::declval<member_map &>(), std::declval<entity const &>())), int const &>);
}

TEST_CASE("member_arg maps objects to their data members", "[property][property_map][member]")
{
  entity e{};
  auto map = heim::make_property_map(heim::member_arg, &entity::hp);
  REQUIRE(&prop::get(map, e) == &e.hp);
  prop::set(map, e, 73);
  REQUIRE(e.hp == 73);
  entity const &constant = e;
  REQUIRE(&prop::get(map, constant) == &e.hp);
}

TEST_CASE("member_arg supports raw and smart pointers to objects", "[property][property_map][member]")
{
  auto map = heim::make_property_map(heim::member_arg, &entity::hp);
  entity e{};
  REQUIRE(&prop::get(map, &e) == &e.hp);
  prop::set(map, &e, 25);
  REQUIRE(e.hp == 25);

  auto ptr = std::make_unique<entity>();
  REQUIRE(prop::get(map, ptr) == 100);
  prop::set(map, ptr, 11);
  REQUIRE(ptr->hp == 11);

  auto reference = std::ref(e);
  prop::set(map, reference, 21);
  REQUIRE(e.hp == 21);
}

TEST_CASE("member_arg adapts reference-returning member functions", "[property][property_map][member]")
{
  entity e{};
  auto map = heim::make_property_map(heim::member_arg, static_cast<int &(entity::*)()>(&entity::health));
  REQUIRE(&prop::get(map, e) == &e.hp);
  prop::set(map, e, 39);
  REQUIRE(e.hp == 39);
  static_assert(!prop::readable_map_for<decltype(map) &, entity const &>);
}

TEST_CASE("member_arg supports const member functions that return values", "[property][property_map][member]")
{
  auto map = heim::make_property_map(heim::member_arg, &entity::strength);
  entity const e{10, 20};
  REQUIRE(prop::get(map, e) == 30);
  static_assert(!prop::writable_map_for<decltype(map) &, entity &, int>);
  static_assert(std::same_as<decltype(prop::get(map, e)), int>);
}
